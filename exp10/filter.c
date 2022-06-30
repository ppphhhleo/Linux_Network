/*
 * This code was compiled and tested on Ubuntu 18.04.2
 * with kernel version 4.15.0
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rui");
MODULE_DESCRIPTION("A simple example netfilter module.");
MODULE_VERSION("0.0.1");
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
typedef struct band_status
{
    /* IP发送禁止，IP地址，当为0时，未设置 */
    unsigned int band_ip;

    /* 端口禁止，当协议和端口均为0时，未设置 */
    nf_bandport band_port;

    /* 是否允许ping回显响应，为0时响应，为1时禁止 */
    unsigned char band_ping;
} band_status;

/* 初始化绑定状态 */   
static band_status b_status;

/* nf sock 选项扩展操作*/
static int nf_sockopt_set(struct sock *sock,
         int cmd,
         void __user *user,
         unsigned int len)
{
    int    ret = 0;
    struct band_status  status;

    /* 权限检查 */
    if (!capable(CAP_NET_ADMIN))        /*没有足够权限*/
    {
    ret = -EPERM;
    goto ERROR;
    }
    /* 从用户空间复制数据*/
    ret = copy_from_user(&status, user, len);
    if (ret != 0)                        /*复制数据失败*/
    {
    ret = -EINVAL;
    goto ERROR;
    }

    /* 命令类型 */
    switch (cmd)
    {
    case SOE_BANDIP:                                /*禁止IP协议*/
    /* 设置禁止IP协议 */
    if (IS_BANDIP(status)){              /*设置禁止IP协议*/
        b_status.band_ip = status.band_ip;
        printk(KERN_ALERT "IP地址%d",b_status.band_ip);
    }

    else{
                                        /*取消禁止*/
        b_status.band_ip = 0;
         printk(KERN_ALERT "IP地址为0");
    }

    break;
    case SOE_BANDPORT:                              /*禁止端口*/
    /* 设置端口禁止和相关的协议类型 */
    if (IS_BANDPORT_TCP(status))        /*禁止TCP*/
    {
        b_status.band_port.protocol = IPPROTO_TCP;
        b_status.band_port.port  = status.band_port.port;
    } else if (IS_BANDPORT_UDP(status)) /*禁止UDP*/
    {
        b_status.band_port.protocol = IPPROTO_UDP;
        b_status.band_port.port  = status.band_port.port;
    }
    /*其他*/
    else{
        b_status.band_port.protocol = 0;
        b_status.band_port.port  = 0;
    }

    break;
    case SOE_BANDPING:                      /*禁止ping*/
    if (IS_BANDPING(status))    /*禁止PING*/
    {
        b_status.band_ping = 1;
    } else{                        /*取消禁止*/
        b_status.band_ping = 0;
    }

    break;
    default:                                /*其他为错误命令*/
    ret = -EINVAL;
    break;
    }

ERROR:
    return(ret);
}
//   int (*get)(struct sock *sk, int optval, void __user *user, int *len);
// #ifdef CONFIG_COMPAT
//   int (*compat_get)(struct sock *sk, int optval,
//       void __user *user, int *len);

/* nf sock 操作扩展命令操作*/
static int nf_sockopt_get(struct sock *sock,
         int cmd,
         void __user *user,
         int *len)
{
    int ret = 0;
    // printk(KERN_ALERT "%d",sizeof(struct band_status));

    /* 权限检查*/
    if (!capable(CAP_NET_ADMIN)) /*没有权限*/
    {
    ret = -EPERM;
    goto ERROR;
    }

    /* 将数据从内核空间复制到用户空间 */
    switch (cmd)
    {
    case SOE_BANDIP:
    case SOE_BANDPORT:
    case SOE_BANDPING:
    // printk(KERN_ALERT "1");
    /*复制数据*/
    ret = copy_to_user(user, &b_status, *len);
    // printk(KERN_ALERT "2");
    if (ret != 0)  /*复制数据失败*/
    {
        ret = -EINVAL;
        goto ERROR;
    }
    break;
    default:
    ret = -EINVAL;
    break;
    }

ERROR:
    return(ret);
}


static unsigned int nf_hook_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    if (!skb)
    return(NF_ACCEPT);

    iph = ip_hdr(skb);
    unsigned int  dest_ip  = iph->daddr;

    if(iph->protocol==IPPROTO_ICMP){                                          /*ICMP协议*/
    /*丢弃ICMP报文*/
        if (IS_BANDPING(b_status))                        /*设置了禁止PING操作*/
        {
            // 注意是大端对齐 字节序
            printk(KERN_ALERT "DROP ICMP packet from %d.%d.%d.%d\n",
            (dest_ip & 0x000000ff) >> 0,
            (dest_ip & 0x0000ff00) >> 8,
            (dest_ip & 0x00ff0000) >> 16,
            (dest_ip & 0xff000000) >> 24);

            return(NF_DROP);                                /*丢弃该报文*/
        }
    }
   
    return(NF_ACCEPT);
}


/* 接收数据、 过滤协议及其端口、禁止ICMP */

/* 在LOCAL_IN挂接钩子 */
static unsigned int nf_hook_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    if (!skb)
    return(NF_ACCEPT);

    iph = ip_hdr(skb);
    unsigned int  src_ip  = iph->saddr;
    if (IS_BANDIP(b_status))                    /*判断是否禁止IP协议*/
    {
        /* TODO ? ip->saddr or ip->daddr */
        if (b_status.band_ip == iph->saddr)  /*接收报文的源IP地址符合*/
        {
            printk(KERN_ALERT "DROP IP packet from %d.%d.%d.%d\n",
            (src_ip & 0x000000ff) >> 0,
            (src_ip & 0x0000ff00) >> 8,
            (src_ip & 0x00ff0000) >> 16,
            (src_ip & 0xff000000) >> 24);
            return(NF_DROP);                /*丢弃该网络报文*/
        }
    }

    
    struct tcphdr *tcph = NULL;
    struct udphdr *udph = NULL;
    unsigned short port_local  = ntohs(b_status.band_port.port);
    switch (iph->protocol)                                        /*IP协议类型*/
    {
    case IPPROTO_TCP:                                              /*TCP协议*/
    /*丢弃禁止端口的TCP数据*/
    if (IS_BANDPORT_TCP(b_status))
    {
        tcph = tcp_hdr(skb);                           /*获得TCP头*/
        /*端口匹配*/
        if (tcph->dest == b_status.band_port.port)
        {
            printk(KERN_ALERT "DROP TCP port %d IP packet from %d.%d.%d.%d\n",
            port_local,
            (src_ip & 0x000000ff) >> 0,
            (src_ip & 0x0000ff00) >> 8,
            (src_ip & 0x00ff0000) >> 16,
            (src_ip & 0xff000000) >> 24);
            return(NF_DROP);                        /*丢弃该数据*/
        }
    }
    break;
    case IPPROTO_UDP:                                              /*UDP协议*/
    /*丢弃UDP数据*/
    if (IS_BANDPORT_UDP(b_status))                      /*设置了丢弃UDP协议*/
    {
        udph = udp_hdr(skb);                           /*UDP头部*/
        
        if (udph->dest == b_status.band_port.port)    /*UDP端口判定*/
        {
            printk(KERN_ALERT "DROP UDP port %d IP packet from %d.%d.%d.%d\n",
            port_local,
            (src_ip & 0x000000ff) >> 0,
            (src_ip & 0x0000ff00) >> 8,
            (src_ip & 0x00ff0000) >> 16,
            (src_ip & 0xff000000) >> 24);
            return(NF_DROP);                        /*丢弃该数据*/
        }
    }

    break;

    default:
    break;
    }

    return(NF_ACCEPT);
}


/* 初始化nf套接字选项 */
static struct nf_sockopt_ops nfsockopt = {
 .pf  = PF_INET,
 .set_optmin = SOE_BANDIP,
 .set_optmax = SOE_BANDIP+3,
 .set = nf_sockopt_set,
 .get_optmin = SOE_BANDIP,
 .get_optmax = SOE_BANDIP+3,
 .get = nf_sockopt_get,
};



static struct nf_hook_ops nf_ops_in = {
    /* Initialize netfilter hook */
    .hook  = (nf_hookfn *) nf_hook_in,    /* hook function */
    .hooknum  = NF_INET_LOCAL_IN,            /* received packets */
    .pf  = PF_INET,                     /* IPv4 */
    .priority = NF_IP_PRI_FIRST             /* max hook priority */
};

static struct nf_hook_ops nf_ops_out = {
    /* Initialize netfilter hook */
    .hook  = (nf_hookfn *) nf_hook_out,   /* hook function */
    .hooknum  = NF_INET_LOCAL_OUT,            /* received packets */
    .pf  = PF_INET,                     /* IPv4 */
    .priority = NF_IP_PRI_FIRST              /* max hook priority */
};


static int __init ban_init(void)
{
    b_status.band_ip = 0;
    b_status.band_port.protocol=IPPROTO_TCP;
    b_status.band_port.port=htons(80);
    b_status.band_ping=1;
    nf_register_net_hook(&init_net, &nf_ops_in);
    nf_register_net_hook(&init_net, &nf_ops_out);
    nf_register_sockopt(&nfsockopt);      /*注册扩展套接字选项*/
    printk(KERN_ALERT "netfilter example 2 install successfully\n");
    return NF_SUCCESS;
}


static void __exit ban_exit(void)
{
    nf_unregister_net_hook(&init_net, &nf_ops_in);
    nf_unregister_net_hook(&init_net, &nf_ops_out);
    nf_unregister_sockopt(&nfsockopt);      /*注册扩展套接字选项*/
    printk(KERN_ALERT "netfilter example 2 clean successfully\n");

}


module_init(ban_init);
module_exit(ban_exit);