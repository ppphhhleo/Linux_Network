# Linux_Network
Linux 网络程序设计。
* 进程双向通信，消息队列，进程通信；
* 多线程（线程创建、销毁、信号量、文件操作，实现生产者消费者线程，并用文件记录过程）；
* 网络抓包，wireshark
* 套接字编程，TCP客户端和服务端交互
* 多播，广播
* ARP协议
* netfilter 框架


## Experiment 2 进程双向通信：消息队列
实现了单个队列和双队列进程通信
* 所遇到问题：如何在子进程中直接结束整个进程

***
## Experiment 3 多线程生产消费
综合利用线程创建、线程销毁、信号量以及文件操作等函数，实现生产者线程、消费者线程，并将生产和消费的过程在文件中记录并显示出来  
    编译：
    ```
    gcc -o 22cp.c 22cp -lphtread
    ```

 ***

## Experiment 5 套接字编程实践
掌握Socket编程的基础知识，了解服务器端和客户端编程的模型，掌握基本的Socket函数，能够实现TCP环境下客户端与服务器端交互的例子。

* 请查看makefile
* media.txt 为客户端发送的内容，server.txt为服务器端的接收记录
* tcp_client.c为客户端连接，tcp_server.c为服务器连接， tcp_process.c为通信处理
* 先运行服务器端：
    ```
    ./server
    ```
* 再运行客户端：
    ```
    ./client
    ```
* 中断：ctrl + C

***

## Experiment 6 UDP框架及网络功能函数

熟悉UDP编程框架，掌握常用网络功能函数的使用，能够进行地址、域名和转换。  

***


## Experiment 7 多播客户端与服务器端实验
了解单播、广播和多播的区别，熟悉多播的实现原理和编程框架，掌握多播服务器和客户端的功能区分。

* 首先运行客户接收端:
    ```cmd
    ./r
    ```
* 其次运行广播服务器
    ```cmd
    ./s
    ```
* rec.c 和 rec2.c 均属于客户接收端

***

## Experiment 8 数据链路层数据获取及ARP协议

熟悉数据链路层数据的获取方法，能够从数据链路层获取网络层、传输层和应用层的数据，掌握ARP协议。


* 可开多台主机，获取IP地址进行测试
* 本次使用了多台虚拟机IP
* ARP攻击

***

## Experiment 9 Linux系统中的Ping程序分析
分析PING原理。

* 可执行程序 ping
* 编译：
    ```cmd
    g++ ping.cpp -o ping -lpthread
    ```
***

## Experiment 10 Netfilter框架编程

了解NetFilter框架，掌握Linux内核模块编程的方法，熟悉Netfilter框架进行数据拦截的方法，能够利用Netfilter框架实现网络数据包的控制。
实现了基于IP、端口等。

* 注意测试环境：**Ubuntu 18.04，内核版本：linux-headers-5.4.0-117-generic**
* 执行init.sh 脚本，注意初次运行无需去除内核模块；之后再次编译之前，需要去除内核模块。
* 查看Makefile，编辑生成的可执行程序名