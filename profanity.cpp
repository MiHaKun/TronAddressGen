#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <set>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#include <OpenCL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#else
#include <CL/cl.h>
#include <CL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#endif

#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV 0x4009

#include "Dispatcher.hpp"
#include "ArgParser.hpp"
#include "Mode.hpp"
#include "help.hpp"
#include "kernel_profanity.hpp"
#include "kernel_sha256.hpp"
#include "kernel_keccak.hpp"

std::string readFile(const char *const szFilename)
{
	std::ifstream in(szFilename, std::ios::in | std::ios::binary);
	std::ostringstream contents;
	contents << in.rdbuf();
	return contents.str();
}

std::vector<cl_device_id> getAllDevices(cl_device_type deviceType = CL_DEVICE_TYPE_GPU)
{
	std::vector<cl_device_id> vDevices;

	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, NULL, &platformIdCount);

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), NULL);

	for (auto it = platformIds.cbegin(); it != platformIds.cend(); ++it)
	{
		cl_uint countDevice;
		clGetDeviceIDs(*it, deviceType, 0, NULL, &countDevice);

		std::vector<cl_device_id> deviceIds(countDevice);
		clGetDeviceIDs(*it, deviceType, countDevice, deviceIds.data(), &countDevice);

		std::copy(deviceIds.begin(), deviceIds.end(), std::back_inserter(vDevices));
	}

	return vDevices;
}

template <typename T, typename U, typename V, typename W>
T clGetWrapper(U function, V param, W param2)
{
	T t;
	function(param, param2, sizeof(t), &t, NULL);
	return t;
}

template <typename U, typename V, typename W>
std::string clGetWrapperString(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	char *const szString = new char[len];
	function(param, param2, len, szString, NULL);
	std::string r(szString);
	delete[] szString;
	return r;
}

template <typename T, typename U, typename V, typename W>
std::vector<T> clGetWrapperVector(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	len /= sizeof(T);
	std::vector<T> v;
	if (len > 0)
	{
		T *pArray = new T[len];
		function(param, param2, len * sizeof(T), pArray, NULL);
		for (size_t i = 0; i < len; ++i)
		{
			v.push_back(pArray[i]);
		}
		delete[] pArray;
	}
	return v;
}

std::vector<std::string> getBinaries(cl_program &clProgram)
{
	std::vector<std::string> vReturn;
	auto vSizes = clGetWrapperVector<size_t>(clGetProgramInfo, clProgram, CL_PROGRAM_BINARY_SIZES);
	if (!vSizes.empty())
	{
		unsigned char **pBuffers = new unsigned char *[vSizes.size()];
		for (size_t i = 0; i < vSizes.size(); ++i)
		{
			pBuffers[i] = new unsigned char[vSizes[i]];
		}

		clGetProgramInfo(clProgram, CL_PROGRAM_BINARIES, vSizes.size() * sizeof(unsigned char *), pBuffers, NULL);
		for (size_t i = 0; i < vSizes.size(); ++i)
		{
			std::string strData(reinterpret_cast<char *>(pBuffers[i]), vSizes[i]);
			vReturn.push_back(strData);
			delete[] pBuffers[i];
		}

		delete[] pBuffers;
	}

	return vReturn;
}

unsigned int getUniqueDeviceIdentifier(const cl_device_id &deviceId)
{
	cl_int bus_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_BUS_ID_NV);
	cl_int slot_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_SLOT_ID_NV);
	return (bus_id << 16) + slot_id;
}

template <typename T>
bool printResult(const T &t, const cl_int &err)
{
	std::cout << ((t == NULL) ? toString(err) : "Done") << std::endl;
	return t == NULL;
}

bool printResult(const cl_int err)
{
	std::cout << ((err != CL_SUCCESS) ? toString(err) : "Done") << std::endl;
	return err != CL_SUCCESS;
}

std::string getDeviceCacheFilename(cl_device_id &d, const size_t &inverseSize)
{
	const auto uniqueId = getUniqueDeviceIdentifier(d);
	return "cache-opencl." + toString(inverseSize) + "." + toString(uniqueId);
}

int main(int argc, char **argv)
{
	try
	{
		ArgParser argp(argc, argv);
		bool bHelp = false;

		std::string matchingInput;
		std::string outputFile;
		// localhost test post url
		std::string postUrl = "http://127.0.0.1:7002/api/address";
		std::vector<size_t> vDeviceSkipIndex;
		size_t worksizeLocal = 64;
		size_t worksizeMax = 0;
		bool bNoCache = false;
		size_t inverseSize = 255;
		size_t inverseMultiple = 16384;
		size_t prefixCount = 0;
		size_t suffixCount = 6;
		size_t quitCount = 0;

		argp.addSwitch('h', "help", bHelp);
		argp.addSwitch('m', "matching", matchingInput);
		argp.addSwitch('w', "work", worksizeLocal);
		argp.addSwitch('W', "work-max", worksizeMax);
		argp.addSwitch('n', "no-cache", bNoCache);
		argp.addSwitch('o', "output", outputFile);
		argp.addSwitch('p', "post", postUrl);
		argp.addSwitch('i', "inverse-size", inverseSize);
		argp.addSwitch('I', "inverse-multiple", inverseMultiple);
		argp.addSwitch('b', "prefix-count", prefixCount);
		argp.addSwitch('e', "suffix-count", suffixCount);
		argp.addSwitch('q', "quit-count", quitCount);
		argp.addMultiSwitch('s', "skip", vDeviceSkipIndex);

		if (!argp.parse())
		{
			std::cout << "error: bad arguments, try again :<" << std::endl;
			return 1;
		}

		if (bHelp)
		{
			std::cout << g_strHelp << std::endl;
			return 0;
		}

		if (matchingInput.empty())
		{
			std::cout << "error: matching file must be specified :<" << std::endl;
			return 1;
		}

		if (prefixCount < 0)
		{
			prefixCount = 0;
		}

		if (prefixCount > 10)
		{
			std::cout << "error: the number of prefix matches cannot be greater than 10 :<" << std::endl;
			return 1;
		}

		if (suffixCount < 0)
		{
			suffixCount = 6;
		}

		if (suffixCount > 10)
		{
			std::cout << "error: the number of suffix matches cannot be greater than 10 :<" << std::endl;
			return 1;
		}

		Mode mode = Mode::matching(matchingInput);

		if (mode.matchingCount <= 0)
		{
			std::cout << "error: please check your matching file to make sure the path and format are correct :<" << std::endl;
			return 1;
		}

		mode.prefixCount = prefixCount;
		mode.suffixCount = suffixCount;

		std::vector<cl_device_id> vFoundDevices = getAllDevices();
		std::vector<cl_device_id> vDevices;
		std::map<cl_device_id, size_t> mDeviceIndex;

		std::vector<std::string> vDeviceBinary;
		std::vector<size_t> vDeviceBinarySize;
		cl_int errorCode;
		bool bUsedCache = false;

		std::cout << "Devices:" << std::endl;
		for (size_t i = 0; i < vFoundDevices.size(); ++i)
		{
			if (std::find(vDeviceSkipIndex.begin(), vDeviceSkipIndex.end(), i) != vDeviceSkipIndex.end())
			{
				continue;
			}
			cl_device_id &deviceId = vFoundDevices[i];
			const auto strName = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_NAME);
			const auto computeUnits = clGetWrapper<cl_uint>(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_COMPUTE_UNITS);
			const auto globalMemSize = clGetWrapper<cl_ulong>(clGetDeviceInfo, deviceId, CL_DEVICE_GLOBAL_MEM_SIZE);
			bool precompiled = false;

			if (!bNoCache)
			{
				std::ifstream fileIn(getDeviceCacheFilename(deviceId, inverseSize), std::ios::binary);
				if (fileIn.is_open())
				{
					vDeviceBinary.push_back(std::string((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>()));
					vDeviceBinarySize.push_back(vDeviceBinary.back().size());
					precompiled = true;
				}
			}

			std::cout << "  GPU-" << i << ": " << strName << ", " << globalMemSize << " bytes available, " << computeUnits << " compute units (precompiled = " << (precompiled ? "yes" : "no") << ")" << std::endl;
			vDevices.push_back(vFoundDevices[i]);
			mDeviceIndex[vFoundDevices[i]] = i;
		}

		if (vDevices.empty())
		{
			return 1;
		}

		std::cout << std::endl;
		std::cout << "OpenCL:" << std::endl;
		std::cout << "  Context creating ..." << std::flush;
		auto clContext = clCreateContext(NULL, vDevices.size(), vDevices.data(), NULL, NULL, &errorCode);
		if (printResult(clContext, errorCode))
		{
			return 1;
		}

		cl_program clProgram;
		if (vDeviceBinary.size() == vDevices.size())
		{
			// Create program from binaries
			bUsedCache = true;

			std::cout << "  Binary kernel loading..." << std::flush;
			const unsigned char **pKernels = new const unsigned char *[vDevices.size()];
			for (size_t i = 0; i < vDeviceBinary.size(); ++i)
			{
				pKernels[i] = reinterpret_cast<const unsigned char *>(vDeviceBinary[i].data());
			}

			cl_int *pStatus = new cl_int[vDevices.size()];

			clProgram = clCreateProgramWithBinary(clContext, vDevices.size(), vDevices.data(), vDeviceBinarySize.data(), pKernels, pStatus, &errorCode);
			if (printResult(clProgram, errorCode))
			{
				return 1;
			}
		}
		else
		{
			// Create a program from the kernel source
			std::cout << "  Kernel compiling ..." << std::flush;

			// const std::string strKeccak = readFile("keccak.cl");
			// const std::string strSha256 = readFile("sha256.cl");
			// const std::string strVanity = readFile("profanity.cl");
			// const char *szKernels[] = {strKeccak.c_str(), strSha256.c_str(), strVanity.c_str()};

			const char *szKernels[] = {kernel_keccak.c_str(), kernel_sha256.c_str(), kernel_profanity.c_str()};
			clProgram = clCreateProgramWithSource(clContext, sizeof(szKernels) / sizeof(char *), szKernels, NULL, &errorCode);
			if (printResult(clProgram, errorCode))
			{
				return 1;
			}
		}

		// Build the program
		std::cout << "  Program building ..." << std::flush;
		const std::string strBuildOptions = "-D PROFANITY_INVERSE_SIZE=" + toString(inverseSize) + " -D PROFANITY_MAX_SCORE=" + toString(PROFANITY_MAX_SCORE);
		if (printResult(clBuildProgram(clProgram, vDevices.size(), vDevices.data(), strBuildOptions.c_str(), NULL, NULL)))
		{
			return 1;
		}

		// Save binary to improve future start times
		if (!bUsedCache && !bNoCache)
		{
			std::cout << "  Program saving ..." << std::flush;
			auto binaries = getBinaries(clProgram);
			for (size_t i = 0; i < binaries.size(); ++i)
			{
				std::ofstream fileOut(getDeviceCacheFilename(vDevices[i], inverseSize), std::ios::binary);
				fileOut.write(binaries[i].data(), binaries[i].size());
			}
			std::cout << "Done" << std::endl;
		}

		std::cout << std::endl;

		Dispatcher d(clContext, clProgram, mode, worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax, inverseSize, inverseMultiple, quitCount, outputFile, postUrl);

		for (auto &i : vDevices)
		{
			d.addDevice(i, worksizeLocal, mDeviceIndex[i]);
		}

		d.run();
		clReleaseContext(clContext);
		return 0;
	}
	catch (std::runtime_error &e)
	{
		std::cout << "std::runtime_error - " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception occured" << std::endl;
	}

	return 1;
}
