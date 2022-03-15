/*************************************************************************
	> File Name: a.c
	> Author: 
	> Mail: 
	> Created Time: Wed 09 Mar 2022 02:46:25 AM PST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include<signal.h>
#define MAX_TEXT 512
int main(){
      
	int ret = -1;
	int msg_flags, msg_id;/*创建消息队列函数所用的标志位以及消息队列的id号*/
	key_t key;/*队列的键值*/
	struct msgmbuf{
      /*消息的缓冲区结构*/
		long mtype; // 注意long与int类型的字长问题
		char mtext[MAX_TEXT];
		};
	struct msgmbuf msg_mbuf;/*创建消息缓冲区*/
	
	int msg_sflags,msg_rflags;/*收发消息函数所用的标志位*/
	char *msgpath = "/usr/bin/";/*消息key产生所用的路径*/
    key = ftok(msgpath,'b');/*产生key*/
	if(key != -1)/*产生key成功*/
	{
      
		printf("B 成功建立KEY\n");		
	}
	else/*产生key失败*/
	{
      
		printf("建立KEY失败\n");		
	}
	msg_flags = IPC_CREAT;//|IPC_EXCL;		/*设置创建消息的标志位*/
	msg_id = msgget(key, msg_flags|0666);	/*建立消息队列*/
	if( -1 == msg_id )
	{
      
		printf("消息建立失败\n");
		return 0;		
	}	
    char buffer[MAX_TEXT];
	pid_t pid;
	pid = fork();/*通过fork()创建子进程，主进程进行发消息，子进程进行收消息*/

	while(1){
      
		if(pid != 0){
      /*主进程*/
			msg_sflags = IPC_NOWAIT;
			msg_mbuf.mtype = 11;/*发送消息的类型为10，另一个进程收消息的类型应为10*/
			sleep(1);
			printf("input: ");
        	fgets(buffer, BUFSIZ, stdin);
        	strcpy(msg_mbuf.mtext, buffer);
			ret = msgsnd(msg_id, &msg_mbuf, 11, msg_sflags);/*发送消息*/
		
			if(strncmp(buffer,"end",3) == 0)/*如果前三个字符为end，则跳出循环*/
				break;
			printf("\033[1;37;43m B Send:\033[0m \033[1;33m %s\033[0m\n", msg_mbuf.mtext);	
			if( -1 == ret) {
            	return 0;
				printf("发送消息失败\n");		
			}
		} else {
      /*子进程*/
			sleep(1);
			msg_mbuf.mtype = 10;/*收消息的类型为11，另一个进程发消息的类型应为11*/
			msg_rflags = IPC_NOWAIT;//|MSG_NOERROR;
			ret = msgrcv(msg_id, &msg_mbuf,MAX_TEXT,10,msg_rflags);/*接收消息*/
			if( -1 == ret)
			{
      
				/*可添加出错处理等*/
			}
			else
			{
                if(strncmp(msg_mbuf.mtext, "end", 3) == 0) {
			
                    printf("\n\033[1;37;41m Another process has been exit, Exit\033[0m\n");
					kill(pid, SIGKILL);
		    
                }

				printf("\n\033[1;37;46m B Receive :\033[0m \033[1;32m%s\033[0m\n",msg_mbuf.mtext);			
			}
		
		}
	}

	sleep(2);
	ret = msgctl(msg_id, IPC_RMID,NULL);/*删除消息队列*/
	if(-1 == ret)
	{
        return 0;	
	}
	return 0;
}