/*************************************************************************
	> File Name: sen.c
	> Author: 
	> Mail: 
	> Created Time: Mon 16 May 2022 08:03:00 AM PDT
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
#include <sys/signal.h>
//向组播地址为224.0.0.88的组发送信息
void notice() {
    printf("\033[1;37;41m Ctrl+C 服务器中断关闭 \033[0m \n ");
    exit(0);
}
int main() {
    FILE *fd;
    char group_addr[16]="224.0.0.88";
    int socked=socket(AF_INET,SOCK_DGRAM,0);
    if(socked<0)
    {
        perror("socket failed!");
        return 2;
    }

    struct sockaddr_in remote_addr;
    memset(&remote_addr,0,sizeof(remote_addr));

    remote_addr.sin_family=AF_INET;
    remote_addr.sin_addr.s_addr=inet_addr(group_addr);
    remote_addr.sin_port=htons(8888);
    
    char buf[1024];
    int length=0;
    fd = fopen("./media.txt", "r");
    int mnum = 0;
    while(1)
    {
        memset(buf, 0, sizeof(buf));
	sleep(1);
	fgets(buf, 1024, fd);
        if(strlen(buf) == 0) {
	    printf("\033[1;37;44m 文件内容已读取完毕，共发送消息：%d 条\033[0m \n ", mnum);
		length=sendto(socked,buf,strlen(buf),0,(struct sockaddr *)&remote_addr,sizeof(remote_addr));break;}
        if(strlen(buf) > 0) {
		buf[strlen(buf)-1] = '\0';
        	length=sendto(socked,buf,strlen(buf),0,(struct sockaddr *)&remote_addr,sizeof(remote_addr));
		mnum += 1;
        	printf("\033[1;37;46m Send Message: \033[0m \033[1;37;40m %s \033[0m \n", buf);
	}
    }
    close(socked);
    close(fd);
    return 0;
}

