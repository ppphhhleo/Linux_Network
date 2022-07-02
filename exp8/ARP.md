# Exp8 数据链路层数据获取及ARP协议


## 实验目的及实验内容
==================

熟悉数据链路层数据的获取方法，能够从数据链路层获取网络层、传输层和应用层的数据，掌握ARP协议。发送ARP请求报文

### 1.1 获取数据链路层数据
----------------------

Linux下数据链路层的访问通常是通过编写内核驱动程序来实现，在应用层使用SOCK\_PACKET类型的协议族可以实现部分功能。建立套接字选择SOCK\_PACKET类型，内核将不对网络数据进行处理而直接交付给用户，数据直接从网卡的协议栈交给用户。

>   **Socket (AF\_INET, SOCK\_PACKET, htons(0x0003))**

其中AF\_INET表示因特网协议族，SOCK\_PACKET表示截取数据帧的层次在物理层，网络协议栈不对数据处理，0x0003表示截取的数据帧类型不确定，处理所有包。

### **1.2 设置套接口以捕获链路帧**

使用ioctl()
的SIOCGIFFLAGS和SIOCSIFFLAGS命令，用来取出和写入网络接口的标志设置。修改网络接口标志的时候，首先取出标志位，再将其与要设置的标志位进行位或运算，将所得结果作为目标标志位写入。

>   **ioctl(sockfd, SIOCGIFHWADDR, (char \*)&req) // 获取MAC地址**

>   **ioctl(sockfd, SIOCGIFADDR, (char \*)&req) // 获取IP地址**

### **1.3 从套接口读取链路帧**

读取数据链路层的数据帧，要清楚以太网的数据结构，总长度最大为1518字节，最小帧长为64字节，其中目标地址MAC为6字节，源MAC地址6字节，协议类型2字节，含46\~1500字节数据，尾部4个字节为CRC校验和。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/85d96e4dc1686a7b8e07c97d5cdfabb5.png)

套接字文件描述符建立后，就可以从中读取以上格式的以太网帧。同时建立一个大小为ETH\_FRAME\_LEN的缓冲区，即可实现数据转存。缓冲区与以太网头部对应关系如下：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/51863f93550bd054db04573cddc65729.png)

以此可实现读取数据帧的目标MAC、源MAC地址。

### **1.4 定位IP报头**

获得以太网帧后，当协议为0x0800时，其负载部分为IP协议。IP协议数据结构如下所示：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/5d29f21a13960f898acb8294d046a6f4.png)

以太网帧协议0x0800时，类型为iphdr的结构指针指向帧头后面载荷数据的起始位置，则可以得到IP数据包的报头部分，再根据对应偏移即可得到源IP地址和目标IP地址。

### **1.5 定位TCP报头**

TCP数据包的数据结构如下所示：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/6a3dfcd8bddfe6dd63f73327f66cacda.png)

对于TCP协议，其IP头部协议值为6，通过计算IP头部的长度，可以得到TCP头部地址，即TCP头部为IP头部偏移的四倍。定位得到TCP报头，再根据对应偏移即可得到源端口和目的端口。

### **1.6 定位UDP报头**

UDP数据结构如下：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/7c9d9d9073b3939bcd2efabf1764e411.png)

对于UDP协议，其IP头部协议值为17，通过计算IP头部长度，可以得到UDP头部地址，为IP头部偏移的四倍。定位得到UDP报头，再根据对应偏移即可得到源端口和目的端口。

### **1.7 定位应用层报文**

定位了UDP和TCP头部地址，其中的数据部分即为应用层报文数据，根据TCP和UDP的协议即可获得应用数据。

### **1.8 ARP协议**

ARP协议的全称是 Address Resolution
Protocol(地址解析协议)，它是一个通过用于实现从 IP
地址到MAC地址的映射，即询问**目标IP对应的MAC地址**的一种协议。

在以太网(Ethernet)中，当主机准备发送一个包含目标IP地址的包时，通过查询自身ARP缓存表、或发送ARP请求包来获取目标主机对应的MAC地址，如此就完成了一次地址解析。

包含以太网头部数据的ARP协议数据结构如下所示：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/fe59aae1ad63af2c57c24ffb6c89bd03.png)

ARP协议的结构体和对应的字段含义如下所示：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/75ed340bf679befa8b04a72ad72cd35a.png)

### **1.9 ARP攻击**

ARP攻击的目的是截获主机之间的通信流量，比如一台计算机和网关之间的通信，让原本不经过我们的流量，将其引导过来，然后就可以进行诸如嗅探(sniff)之类的工作。通过发送ARP响应包给对方（即使对方并没有发送过ARP请求），以此刷新对方的ARP缓存表，让对方通信的原目标MAC地址变为我方的MAC地址。

本次实验，将使用ARP发送请求数据，并接收响应数据包，与ARP表的信息进行对照验证。同时，模拟实现ARP攻击，发送ARP欺骗包。

### **2.1 ARP请求**

#### **2.1.1 获取本机的MAC地址和IP地址**

首先指定SOCK\_DGRAM参数创建原始套接字，获取本机所在网络信息（eth0），并使用ioctl()函数分别指定SIOCGIFADDR和SIOCGIFHWADDR参数获取IP地址和MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/e0727406536c1f464c86d28f981f79dc.png)

#### **2.1.2 设置ARP帧格式并填充**

设置要发送的报文格式，就是将以太网头部和ARP包组合，本次实验根据RFC
0826以太网地址解析协议，定义\_Ether\_pkg，包含以太网头和ARP协议相关的数据结构。将获取的网络类型、本地IP地址和本地MAC地址，以及对应的目标IP地址和目标MAC地址，填入数据包：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/a0d502817ff1bed78b0070dfd434e90f.png)

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/edf3473ad0bf9d2ae6bb70d857089595.png)

#### **2.1.3 发送ARP请求报文**

指定SOCK\_PACKET参数，建立套接字，通过sendto()函数发送请求报文。本次实验，将依次发送查询192.168.181.1
\~ 192.168.181.255 的IP地址所对应的MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/42508482c0ad96f191e5d6711eacccaf.png)

#### **2.1.4 接收ARP响应报文**

发送ARP请求后，本机将进行监听，使用recvfrom()函数接收套接字返回的消息。若收到符合的ARP响应报文，将进行解析，输出发送ARP响应报文的源IP地址和源MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/1334f5b9bd4ed352ca55b8a4993364e3.png)

### **2.2 ARP欺骗**

#### **2.2.1 确定IP和MAC地址**

首先通过嗅探获取要攻击的目标主机MAC地址和IP地址，设置好本地网关IP地址和本地MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/7d4e47e1b0b1b776d10ff0c9170faa9a.png)

#### **2.2.2 伪造ARP响应帧**

该步骤与上述类似，ARP帧就是作为攻击者要发送的数据包，组合以太网头部和ARP包，这里可以使用两个预定义好的结构体。组装成frame[]格式，前14字节为以太网头，后28字节为ARP包。

将上述确定的地址，写入ARP帧中，将源IP地址设置为网关IP，却将源MAC地址设置为本地MAC地址，伪造成要发送给攻击目标的ARP响应包，从而将数据包欺骗发送至本地MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/f6166bec3ec45034a1b4f7ac9e42b0aa.png)

#### **2.2.3 发送伪造ARP响应报文**

创建套接字，指定参数为AF\_PACKET，保证数据链路数据的发送连接。并指定sockaddr\_ll作为sendto()函数的参数，作为网络接口。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/2d1e4477bfbc784db6a5147f5676843c.png)

使用sendto()函数，将此前伪造的ARP响应包发送：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/19c6c4c7a68cc3a334b9a2d6e38db1be.png)

## **实验结果**

### 3.1 发送ARP请求

本机所在子网为192.168.181.0/24，本次实验首先获取本地MAC地址和本地IP地址，其次向子网的每个IP地址（**192.168.181.1
\~ 192.168.181.255，共255个IP**）发送ARP请求。

**本地IP为192.168.181.158，本地MAC地址为00:0C:29:E0:39:62**

若收到ARP响应，则将该响应报文的源IP地址和源MAC地址输出，ARP响应报文的目标IP和MAC为本地。

输出结果如下所示：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/0e31b6e608fa26e0da0138756440e0af.png)

使用WireShark监控虚拟机发出的数据包，可捕获到多个ARP广播帧，其中192.168.181.1、192.168.181.2、192.168.181.254收到ARP请求，并发回请求，告诉了对应的MAC地址。

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/578413d43f2db69c71d9712cde025bf9.png)

WireShark
具体查看192.168.181.254发回的ARP响应帧，可见解析得到的MAC地址，与实验输出结果一致：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/fbbbd6416a6eff4b78ed36f9401ab9c1.png)

使用ARP命令查看ARP缓存表信息，共有三条记录，与输出结果一致：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/18d2f50e4fd42f8427a83e0bf7693c1e.png)

其中**192.168.1.2** 为网关IP地址，其余两个为另外的虚拟机接口。

### 3.2 ARP欺骗

我们对上述192.168.181.254对应的主机，发送伪造的ARP响应报文，将发送方的IP地址设置为网关IP，而发送发的MAC地址设置为本地主机。使用WireShark可捕获到ARP响应报文，解析信息与实验所设置的一致：

![](https://github.com/ppphhhleo/Linux_Network/blob/main/pics/media/2f9f24442c748d5240d10173ce38e716.png)

通过伪造并发送ARP响应报文，实现对攻击目标主机的ARP缓存更新，在实际攻击中，将**需要循环不断发送ARP请求**，放置ARP缓存被其他主机更新（比如网关）。对于ARP攻击，可以设置ARP防火墙来实现防御。
