#ifndef HPP_HELP
#define HPP_HELP

#include <string>

const std::string g_strHelp = R"(
用法: ./TronAddressGen [选项]

  帮助:
    --help              显示帮助信息

  带参数的模式:
    --matching          匹配输入、文件或单个地址。

  匹配配置:
    --prefix-count      最小前缀匹配数量，默认值为 0
    --suffix-count      最小后缀匹配数量，默认值为 6
    --quit-count        当生成的数量大于此值时退出程序，默认值为 0

  设备控制:
    --skip              跳过由索引给定的设备

  输出控制:
    --output            输出结果的文件
    --post              结果要发布到的 URL

示例:

  ./TronAddressGen --matching profanity.txt
  ./TronAddressGen --matching profanity.txt --skip 1
  ./TronAddressGen --matching profanity.txt --prefix-count 1 --suffix-count 8
  ./TronAddressGen --matching profanity.txt --prefix-count 1 --suffix-count 10 --quit-count 1
  ./TronAddressGen --matching profanity.txt --output result.txt
  ./TronAddressGen --matching profanity.txt --post http://127.0.0.1:7002/api
  ./TronAddressGen --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 2 --suffix-count 4 --quit-count 1

关于:

  TronAddressGen 是一个用于 Tron 的虚荣地址生成器: https://tron.network/
  该软件基于以太坊 profanity 进行了修改: https://github.com/johguse/profanity
  请确保您运行的程序是从以下地址下载的: https://github.com/GG4mida/profanity-tron
  作者: telegram -> @jackslowfak
  清理修复：telegram -> @MrMiHa

FBI 警告:

  在使用生成的虚荣地址之前，请始终验证它是否与打印的私钥匹配。
  并始终对地址进行多重签名以确保账户安全。
)";

#endif /* HPP_HELP */
