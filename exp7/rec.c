/*************************************************************************
	> File Name: rec.c
	> Author: 
	> Mail: 
	> Created Time: Mon 16 May 2022 08:03:16 AM PDT
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
// 接收组播地址为224.0.0.88  的信息

int main(int argc,char *argv[]){
    FILE *fa;
    int socked=socket(AF_INET,SOCK_DGRAM,0);
    if(socked<0)
    {
        perror("socket failed!");
        return 2;
    }

    char group[16]="224.0.0.88";
   // char group_1[16]="224.0.0.66";


    struct sockaddr_in local_addr;
    memset(&local_addr,0,sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(8888);

    int ret = bind(socked, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if(ret<0)
    {
        perror("bind failed !");
        return 3;
    }


    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /*
     *
     *  int setsockopt(int sockfd, int level, int optname,
                      const void *optval, socklen_t optlen);
     * param:
     *      optname
     *          * IP_MULTICAST_LOOP 支持多播数据回送
     *          * IP_ADD_MEMBERSHIP 加入多播组
     *          * IP_DROP_MEMBERSHIP 离开多播组
     *      optval
     *          * IP_MULTICAST_LOOP 选项对应传入 unsigned int 来确认是否支持多播数据回送
     *          * IP_ADD_MEMBERSHIP 传入 ip_mreq
     *          * IP_DROP_MEMBERSHIP 传入 ip_mreq
     *
     * */
    ret=setsockopt(socked,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));
    if(ret < 0){
        perror("setsockopt failed !");
        return 3;
    }else
    {
        printf("\033[1;37;44m Setsockopt Success 成功加入广播组 %s\033[0m\n",group);
    }


    char buf[1024];
    int length=0;
    struct sockaddr_in sender;
    socklen_t sender_len=sizeof(sender);
    
    while (1){
       //signal(SIGINT, notice);
        memset(buf, 0, sizeof(buf));
        length=recvfrom(socked, buf, sizeof(buf), 0, (struct sockaddr*)&sender,&sender_len);
	if(strlen(buf) == 0) {printf("\033[1;37;44m DONE \033[0m \n "); break;}
	fa = fopen("./receive.txt", "a");
        fprintf(fa, "\nReceived: ");
        fprintf(fa, buf);
	fprintf(fa, "\n%d bytes altogether\n", strlen(buf));
	fclose(fa);
        //printf("\033[1;37;43m %s %d \033[0m: \033[1;37;40m%s \033[0m\n",inet_ntoa(sender.sin_addr), ntohs(sender.sin_port), buf);
printf("\033[1;37;43m Received From %s: \033[0m \033[1;37;40m%s \033[0m\n",inet_ntoa(sender.sin_addr),  buf);
    }

    setsockopt(socked, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mreq, sizeof(mreq));
    
    close(socked);
    return 0;
}

