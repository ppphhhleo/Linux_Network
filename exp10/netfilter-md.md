# Exp10 Netfilter框架编程

了解NetFilter框架，掌握Linux内核模块编程的方法，熟悉Netfilter框架进行数据拦截的方法，能够利用Netfilter框架实现网络数据包的控制。

-   掌握Linux内核模块的编程方法；

-   掌握Netfilter框架钩子函数设计与实现方法；

-   能够利用Netfilter框架和钩子函数实现以下功能：

-   禁止Ping发送；

-   禁止某个IP地址数据包的接收；

-   禁止某个端口的数据响应；

**实验环境：Ubuntu 18.04，内核版本：linux-headers-5.4.0-117-generic**

### 1.1 Linux内核模块基本结构

Linux 内核模块的基本结构如下图所示，其中各个模块的作用为：

➢
初始化函数：使用命令insmod或者modprobe加载内核模块的时候，会自动调用模块的初始化函数，进行模块的初始化工作，主要是资源的申请；

➢
清除函数：使用命令rmmod卸载内核模块的时候，模块清除函数会自动调用，进行模块退出之前的清理工作，主要是状态重置和资源释放

➢
描述信息声明：内核的一些描述性信息，包括模块作者、模块用途、模块版本号和开源许可证、模块别名等

➢
可导出符号表：将允许导出的符号加到公共内核符号表中，或者使用公共内核符号表来解析加载模块中未定义的符号

➢ 加载参数：定义在内核模块加载时传递的参数

![](media/4be18f7178fc68f6d446af6bb83352f3.png)

**内核模块基本结构**

### 1.2 模块加载和卸载

内核模块的**加载过程**分为用户空间动作和内核空间动作：用户空间负责内核模块加载准备；内核空间负责复制、检查和内核模块初始化等工作。

内核模块的**卸载过程**也分为用户空间动作和内核空间动作：用户空间动作负责内核模块卸载准备；内核空间的动作负责卸载前检查、内核模块清理函数的调用、模块清理等工作。

![](media/5c8014b5e5b55c77b21a4bcc434e6cbe.png)

**左图为内核模块加载过程，右图为内核模块卸载过程**

### 1.3 初始化和清除函数

内核模块的初始化函数主要进行初始化工作

初始化函数的主要流程为：（1）使用 insmod 加载内核模块；（2）调用 init\_module
函数；（3）进入到系统调用sys\_init\_module
函数。该函数负责将模块程序复制到内核中，进行必要的检查、分配资源等；（4）调用内核模块中的
init 函数。

![](media/417643678a8300402772a77d857b4ff6.png)

清除函数的主要流程为：（1）使用 rmmod 卸载内核模块；（2）调用 delete\_module
函数；（3）进入到系统调用 sys\_delete\_module
函数。该函数负责卸载前检查、内核清理函数的调用、模块清理等工作。

![](media/a11c9bc635ee5df7f94cd0f752d3febb.png)

### 1.4 内核模块初始化和清理过程的容错处理

Linux内核中经常采用的一种错误处理框架是采用goto语句构建倒置的容错，虽然goto语句备受批评，但是用在这里不论从代码结构还是程序效率考虑都是最佳的选择之一。

### 1.5 Makefile编写

编译内核的Makefile有如下特殊的地方：

（1）指定内核模块的编译文件和头文件路径；

（2）指定编译模块的名称；

（3）给出当前模块的路径。

本次实验，编写如下所示的Makefile：

![](media/50f95bc002b6e0ec8b38ce0d508840ba.png)

## **实验过程分析**
------------

### 2.1 钩子处理规则

钩子函数的返回值可以为NF\_ACCEPT、NF\_DROP、NF\_STOLEN、NF\_QUERE、NF\_REPEAT，分别对应5种处理规则，具体的含义如下所示：

➢ NF\_ACCEPT：继续传递，保持和原来的传输一致

➢ NF\_DROP：丢弃包

➢ NF\_STOLEN：接管包，不再继续传递

➢ NF\_QUERE：队列化包

➢ NF\_REPEAT：再次调用这一个钩子

>   一共包含4个模块，分别是Ping模块、IP 模块、端口模块、动态配置模块.

![](media/df3bb31b635103dc951690081e6cfcf6.png)

##### 2.1.1 PING 模块

Ping模块的主要功能为禁止ping发送。分析要求可知，我们只需要在 NF\_IP\_LOCAL\_OUT
处挂载钩子函数。

![](media/24f75e7a7389e25a3b1cb494c2c16ab3.png)

钩子函数的处理流程如下所示，具体包括：（1）获取IP头部；（2）判断是否为 icmp
协议；（3）若是，则丢弃该数据包。

![](media/c8e1ef38b38d0b6df2d05f47e93ad1a4.png)

代码实现如下：

![](media/e3b46ee2130ddc1ed83a99c2a8e31099.png)

##### 2.1.2 IP模块

IP模块的主要功能为禁止**某个IP地址**数据包的接受。分析要求可知，我们只需要在
**NF\_IP\_LOCAL\_IN**处挂载钩子函数。

![](media/4575e5fbf288df7056369aa13c17dffd.png)

钩子函数的处理流程如下所示，主要包括：（1）获取IP头部；（2）判断源IP是否为要禁止的IP地址，若是，则丢弃该数据包。

![](media/f5e0360116fa5ab81c778da0bfef0a72.png)

IP模块，在LOCAL\_IN挂载钩子，代码实现如下：

![](media/4a362f5bd8e4e3562d90de2c473464dc.png)

##### 2.1.3 端口模块

端口模块的主要功能为**禁止某个端口**的数据响应。分析要求可知，我们只需要在
NF\_IP\_LOCAL\_IN处挂载钩子函数

![](media/7fe3c4026660edf25fd277b9b5424399.png)

钩子函数的处理流程，如下所示，包含：（1）获取IP头部；（2）判断传输层协议类型；（3）如果为
TCP，且目的端口为禁止端口，则丢弃该数据包；（4）如果为
UDP，且目的端口为禁止端口，则丢弃该数据包。

![](media/2d857c618b0e6d169bad76df3d2f2074.png)

端口模块的代码如下所示，可分为TCP丢弃和UDP丢弃：

![](media/3bed2849a2de54bef4c1c4555df4e6ef.png)

![](media/201ec5caba0fee23e2df69e54c1f8257.png)

##### 2.1.4 动态配置模块

动态配置模块的主要功能为通过内核交互实现配置的动态改变。

主要通过getsockopt和 setsockopt扩展实现内核交互。

getsockopt的主要流程如下左图所示，包括：（1）判断用户权限合法性；（2）判断cmd类型合法性；（3）读取配置到用户程序。

setsockopt的主要流程如下右图所示，包括：（1）用户权限合法性判断；（2）将用户输入参数复制到内核空间；（3）根据cmd类型配置相应的模块。

![](media/401d7555e27a8eb5867007834ba2ab34.png)

![](media/30cbd0d08ab6779c7d6e09dc26d35468.png)

**左为getsockopt主要流程，右为setsockopt主要流程**

Getsockopt对应代码如下：

![](media/885fc1daf04e82126e524a950d340365.png)

Setsockopt代码如下：

用户权限检查和从用户空间复制数据：

![](media/a551624fbb9314f0c5bdd33c7d1db24a.png)

配置IP模块：

![./media/image21.png](./media/image21.png)

配置端口模块：

![](media/fd0bbee6fed6c08e0508c390f1691b00.png)

配置PING模块：

![](media/9b980d5b673d2a0d05b7594edf11ce80.png)

所有钩子函数配置完成。

### 2.2 内核初始化

![./media/image24.png](./media/image24.png)

所有函数代码编写在myfilter.c中。

实验结果总结
------------

##### 3.1 编译内核模块


> make

![./media/image25.png](./media/image25.png)

执行命令sudo insmod
myfilter.ko加载内核模块，并通过dmesg查看内核模块输出，可见成功加载内核模块：

![](media/aa053e6f20b86cdc9996fd96198156a3.png)

gcc myfilter.c

得到可执行程序。

### 3.2 内核交互测试

执行交互程序sudo ./a.out，发现能够正常和内核进行交互:

![](media/dc5b5838e6e2bad6b4d8fa13bd6a3f18.png)

**正常进行交互**

### 3.3 PING模块测试

配置PING模块，仅禁止PING模块，不禁止IP和端口。

![](media/9fc69c0a8102bdf662d084b55357c5a9.png)

ping baidu.com PING模块被禁止：

![](media/55b63e13c6f9605b802cf48c1fa0d54a.png)

dmesg 查看内核信息，可见ICMP报文被DROP：

![](media/b6bb60011fef2b7a368dbc7821b0d866.png)

### 3.4 IP模块测试

配置IP模块，设置禁止百度220.181.38.148，不禁止PING和端口：

![](media/1f14b167087277172c1b178b53646485.png)

尝试ping weibo.com ，其IP为123.125.106.158，不属于被禁止的IP，可以PING通：

![](media/27c572d10bab690639aed2e775ca374f.png)

尝试ping baidu.com，IP为被禁止的IP，无法收到对应的ICMP回显报文：

![](media/467c4e219fbf028c2633d4ca05181e41.png)

sudo sendip -p ipv4 -is 220.181.38.148 -d asdfasdf
192.168.181.158，尝试向百度IP发送报文：

![](media/52f874b1bf5586050854f17e45a80f74.png)

dmesg 查看内核信息，可见无论是ICMP响应报文，还是IP响应报文，源地址为被禁止的IP
220.181.38.148，报文均被DROP：

![](media/965ec8868bb3e9d8920c6c22c1869d73.png)

### 3.5 端口模块测试

配置端口模块，设置禁止端口8090，其他端口、PING和IP不禁止：

![](media/b33232ddbc4e8dd33988671fae580025.png)

python3 -m http.server，在本地8000端口运行一个服务器，该端口可正常使用：

![](media/69dc27162f151dfc95cd76c94e4165a2.png)

curl 127.0.0.1:8090，模拟向本地端口向无法正常连接：

![](media/e8a2206f1a80d933e6a682b8069691db.png)

> sudo sendip -p ipv4 -p tcp -td 8090 -d ceshidata 192.168.181.158，sudo sendip -p
ipv4 -p tcp -td 8090 -d ceshidata
192.168.181.158分别模拟向本机192.168.181.158:8090、192.168.181.158:8000发送TCP报文：

![](media/df4c08033613940fec35d844b7f5f293.png)

dmesg 查看内核信息，可见TCP连接中**目的端口8090**的报文被DROP

![](media/4f3129b4dc30b467e053bece2dfe2e51.png)
