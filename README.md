# TronAddressGen
波场的靓号生成器。移植自：profanity。



## 做了啥

- [x] 移除了一些网络相关的代码
- [x] 重新从官方编译生成了OpenCL
- [x] 重制了一个干净的VS2022的工程
- [ ] 准备做点汉化



## 为啥做

群里有人又傻乎乎的找某站买了工具。

辛苦跑了几天，美滋滋的用起来了10位8靓号。

一段时间，都没啥问题

直到，他的账号超过了10kU



## 如何自己编译

1. 下载一个visual studio 2022 ，社区版就行
2. 双击SLN文件
3. 切换编译目标为 Release
4. 生成
5. 代码目录下的x64目录内，release下，会有一个 `TronAddressGen.exe` 
6. 在命令行下，执行 `TronAddressGen.exe --help` 查看帮助。是的，别傻BB的双击。科普计算机基础操作，从我做起
7. 跑吧

```
D:\WorkSpace\TronAddressGen\x64\Release>TronAddressGen.exe --matching TUvvo588wF97jjiBb1Hsqao2ZfhdMrMiHa --suffix-count 6 --quit-count 10
Devices:
  GPU-0: NVIDIA GeForce RTX 2080 Ti, 23621992448 bytes available, 68 compute units (precompiled = yes)

OpenCL:
  Context creating ...Done
  Binary kernel loading...Done
  Program building ...Done

Initializing:
  Should be no longer than 1 minute...
  GPU-0 initialized ...Done
  Initialization time: 2 seconds

Running...
  Before using a generated vanigity address, always verify that it matches the printed private key.
  Please make sure the program you are running is download from: https://github.com/GG4mida/profanity-tron
  And always multi-sign the address to ensure account security.

  Time:    84s Private: bcf1df97e951d2f086f79c31053b90dacd428a10c9c2cbe718ffbe3759b9564d Address:TXdaW81a6xm96DrM5uL2DkBWCL6pMrMiHa
  Time:   170s Private: bcf1df97e922b4e986f79c31053b90dacd428a10c9c2cbe718ffbe3759b96371 Address:TGzBHMukoFRY8c12ZRCHbnNhStQ7MrMiHa
  Time:   236s Private: bcf1df97e91f5e6686f79c31053b90dacd428a10c9c2cbe718ffbe3759b96d7c Address:TGi5GpXjmSLe6e8JgawGV4KrxzV3MrMiHa
  Time:   366s Private: bcf1df97e91f230a86f79c31053b90dacd428a10c9c2cbe718ffbe3759b9815c Address:TXdNGuaFm8pyBwP56tEimgWxgpaNMrMiHa
  Time:   444s Private: bcf1df97e93cc15b86f79c31053b90dacd428a10c9c2cbe718ffbe3759b98d56 Address:TAUihyLBvdBwCKUrGdJuN6z1jbQ1MrMiHa
^Ctal: 164.891 MH/s - GPU0: 164.891 MH/s
```



## 关于

- 作者 米哈( [@MrMiHa](https://t.me/MrMiHa) )是一个苦逼程序员，不是煤场奴工，有问题别太理直气壮的跑来下命令。
- 讨论群组是 : https://t.me/DeveloperTeamGroup 欢迎加入后玩耍
- 随意Fork，记得保留`关于`的内容。
- 感谢 `profanity-tron` 原作者。
