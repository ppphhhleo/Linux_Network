#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include<sys/wait.h>
#define MAX_TEXT 512
int main(){
    int ex = 0;
	int ret = -1;
	int msg_flags, smsg_id ,rmsg_id;/*创建消息队列函数所用的标志位，以及收消息与发消息的队列id*/
	key_t key1,key2;/*队列的键值*/
	struct msgmbuf{/*消息的缓冲区结构*/
		long mtype; // 注意long与int的字长问题
		char mtext[MAX_TEXT];
		};
	struct msgmbuf msg_mbuf;/*创建消息缓冲区*/
	
	int msg_sflags,msg_rflags;/*收发消息函数所用的标志位*/
	char *msgpath1 = "/usr/bin";/*消息key1产生所用的路径*/
	char *msgpath2 = "/usr/bin";/*消息key2产生所用的路径*/

    char buffer[BUFSIZ];

	key1 = ftok(msgpath1,'b');/*产生key1*/
	key2 = ftok(msgpath2,'a');/*产生key2*/
	if(key1 != -1 || key2 != -1)/*产生key成功*/{	
        printf("A 成功建立KEY\n");		
	} else/*产生key失败*/{
		printf("A 建立KEY失败\n");		
	}
	msg_flags = IPC_CREAT;//|IPC_EXCL;		/*设置创建消息的标志位*/
	smsg_id = msgget(key1, msg_flags|0666); /*建立收消息的消息队列*/
	rmsg_id = msgget(key2, msg_flags|0666);	/*建立发消息的消息队列*/
	if( -1 == smsg_id || -1 == rmsg_id) {
		printf("A 消息建立失败\n");
		return 0;		
	}	
    int si, status;
    FILE *fp, *fa;
	pid_t pid,pp;
	pid = fork();/*通过fork()创建子进程，主进程进行发消息，子进程进行收消息*/
    
    fa = fopen("./end.txt", "w");
    fputs("000", fa);
    fclose(fa);
    char buf[4];
	while(1){
		if(pid != 0){/*主进程*/
               
            	fa = fopen("./end.txt", "r");
            	fgets(buf, 4, fa);
            	fclose(fa);
            	// printf("%s\n", buf);
            	if(strncmp(buf, "end", 3) == 0) {
                	break;
            	}
            
            
          si = WEXITSTATUS(status);
               
            if(si == 5) {
                
                break;
            }
            

		    msg_sflags = IPC_NOWAIT;/*当消息队列满了的时候不等待*/
		    msg_mbuf.mtype = 10;/*设置发送的消息类型*/
            
		    sleep(1);
		    
            printf("input: ");
            fgets(buffer, BUFSIZ, stdin);
            strcpy(msg_mbuf.mtext, buffer);
		    //if(strncmp(buffer,"end",3) == 0)/*如果前三个字符为end，则跳出循环*/
            //	break;
		
	ret = msgsnd(smsg_id, &msg_mbuf, MAX_TEXT, msg_sflags);/*发送消息*/
	
		    if( -1 == ret && si != 5){
                		return 0;// B destroy the queue 
			    printf("A 发送消息失败\n");		
		    }

            if(strncmp(buffer, "end", 3) == 0) {
                fa = fopen("./end.txt", "w");
                fputs("end", fa);
                fclose(fa);
                break;
               }
printf("\033[1;37;43m A Send:\033[0m \033[1;33m %s\033[0m\n", msg_mbuf.mtext);
            fp = fopen("./log.txt","a");
            fputs("A: ", fp);
            fputs(buffer, fp);
       
            fclose(fp);
            
        } else {/*子进程*/
			sleep(1);
			// msg_mbuf.mtype = 10;/*设置收消息的类型*/
            msg_mbuf.mtype = 0; // return oldest message
			msg_rflags = IPC_NOWAIT;//|MSG_NOERROR;
			ret = msgrcv(rmsg_id, &msg_mbuf,MAX_TEXT,10,msg_rflags);/*接收消息*/
			if( -1 == ret) {
               
				/*可添加出错处理等*/
			} else {
                 if(strncmp(msg_mbuf.mtext, "end", 3) == 0) {
                    printf("\n \033[1;37;41m Another process has Exited; Enter to Exit\033[0m \n");
                    exit(5);
                    }
               
				
printf("\n\033[1;37;46m A Receive :\033[0m \033[1;32m%s\033[0m\n",msg_mbuf.mtext);	
                
			}

        // printf("A: ");
		}
	}
    
    
    if(si == 5) {
        return 0;
    }
	ret = msgctl(rmsg_id, IPC_RMID,NULL);/*删除收消息的队列*/
	if(-1 == ret)
	{
		printf("A 删除消息失败\n");
		return 0;		
	}
	return 0;
}

