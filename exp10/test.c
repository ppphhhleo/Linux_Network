#include <stdio.h>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/socket.h>
#include <errno.h>
#include <iostream>
#include <cstring>
/* NF初始化状态宏 */
#define NF_SUCCESS  0
#define NF_FAILURE  1

/*快速绑定操作宏*/
/* 判断是否禁止TCP的端口*/
#define IS_BANDPORT_TCP(status) (status.band_port.port != 0 && status.band_port.protocol == IPPROTO_TCP)
/*判断是否禁止UDP端口 */
#define IS_BANDPORT_UDP(status) (status.band_port.port != 0 && status.band_port.protocol == IPPROTO_UDP)
/* 判断端是否禁止 PING */
#define IS_BANDPING(status) (status.band_ping)
/* 判断是否禁止IP协议 */
#define IS_BANDIP(status) (status.band_ip)
using namespace std;

/* cmd命令定义：
 * SOE_BANDIP：IP地址发送禁止命令
 * SOE_BANDPORT：端口禁止命令
 * SOE_BANDPING：ping禁止
 */
#define SOE_BANDIP  0x6001
#define SOE_BANDPORT  0x6002
#define SOE_BANDPING  0x6003
/* 禁止端口结构*/
typedef struct nf_bandport {
    /* band protocol, TCP?UDP */
    unsigned short protocol;

    /* band port */
    unsigned short port;
} nf_bandport;
/* 与用户交互的数据结构 */
typedef struct band_status {
    /* IP发送禁止，IP地址，当为0时，未设置 */
    unsigned int band_ip;

    /* 端口禁止，当协议和端口均为0时，未设置 */
    nf_bandport band_port;

    /* 是否允许ping回显响应，为0时响应，为1时禁止 */
    unsigned char band_ping;
} band_status;

unsigned int IPtoInt(char *str_ip) {
    struct in_addr addr;
    unsigned int int_ip;
    if (inet_aton(str_ip, &addr)) {
        int_ip = ntohl(addr.s_addr);
    }
    return int_ip;
}

static band_status status;

void get_band_status() {
    printf("%s\n", "输出内核信息");
    unsigned short PORT = ntohs(status.band_port.port);
    //unsigned int IP = ntohl(status.band_ip);
    if (IS_BANDPING(status)) {
        printf("\n\n%s\n", "band ping");
    }else{
        printf("%s\n", "no band ping");
    }
    if (IS_BANDPORT_TCP(status)) {
        printf("band TCP port %d\n", PORT);
    }else if(IS_BANDPORT_UDP(status)) {
        printf("band UDP port %d\n", PORT);
    }else{
        printf("%s\n", "no band port");
    }
    if (IS_BANDIP(status)) {
        struct in_addr ip_in_addr;
        ip_in_addr.s_addr = status.band_ip;
        printf("%s\n", inet_ntoa(ip_in_addr));
    }else{
        printf("%s\n", "no band ip");
    }
}

void set_band_status() {
    memset(&status, 0, sizeof(status));
    char ip[20] = {0x00};
    struct in_addr ip_addr;
    char ch;
    printf("输入xx.xx.xx.xx以禁止IP:");
    cin.getline(ip, 20);
    if (ip[0] != 0) {
        inet_aton(ip, &ip_addr);
        status.band_ip = ip_addr.s_addr;
        printf("%d\n",status.band_ip);
        printf("%d\n",ip_addr.s_addr);
        printf("%s\n", inet_ntoa(ip_addr));
    } else {
        status.band_ip = 0;
    }

    printf("禁止ping(Y/other):");
    cin.get(ch);
    if (ch == 'Y' || ch == 'y') {
        status.band_ping = 1;
    } else {
        status.band_ping = 0;
    }

    printf("禁止TCP端口(Y/other):");
    cin.get(ch);
    if (ch == 'Y' || ch == 'y') {
        status.band_port.protocol = IPPROTO_TCP;
    } else {
        printf("禁止UDP端口(Y/other):");
        cin.get(ch);
        if (ch == 'Y' || ch == 'y') {
            status.band_port.protocol = IPPROTO_UDP;
        }
    }

    if (status.band_port.protocol != 0) {
        unsigned short pp;
        printf("输入端口:");
        cin >> pp;
        status.band_port.port = ntohs(pp);
    }

}

int main(int argc, char const *argv[]) {
    setbuf(stdout, NULL);
    socklen_t len = sizeof(status);
    int sockfd;
    printf("打开设备\n");
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) {
        perror("设备打开失败");
        return -1;
    }
    printf("读取设备");
    if (getsockopt(sockfd, IPPROTO_IP, SOE_BANDPING, (void *) &status, &len)) {
        perror("读取失败");
        return -1;
    }
    get_band_status();

    printf("设置内核\n");
    set_band_status();
    if (setsockopt(sockfd, IPPROTO_IP, SOE_BANDPING, &status, len)) {
        perror("设置内核失败\n");
        return -1;
    }
    if (setsockopt(sockfd, IPPROTO_IP, SOE_BANDIP, &status, len)) {
        perror("设置内核失败\n");
        return -1;
    }
    if (setsockopt(sockfd, IPPROTO_IP, SOE_BANDPORT, &status, len)) {
        perror("设置内核失败\n");
        return -1;
    }
    if (getsockopt(sockfd, IPPROTO_IP, SOE_BANDPING, (void *) &status, &len)){
        return -1;
    }
    get_band_status();
    return 0;

}