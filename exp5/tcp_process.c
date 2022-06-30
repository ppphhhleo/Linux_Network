#include <stdio.h>
#include <string.h>
#include <sys/types.h>    
#include <sys/stat.h>    
#include <fcntl.h>
#include <sys/signal.h>

/*客户端的处理过程*/
FILE *fa;

void process_conn_client(int s)
{
	ssize_t size = 0;
	char buffer[1024];							/*数据的缓冲区*/
	char buf[1024];
	FILE *fd;
	fd = fopen("./media.txt", "r");  /* 读取文件内容，fd文件描述符*/ 
	int message_num = 0;
	for(;;){									/*循环处理过程*/
		sleep(1);
		fgets(buf, 1024, fd); /* 从文件中读取内容，放到缓冲区buf中*/ 
		size = strlen(buf);  
		if(size == 1) {  /* 若仅含一个字符，即为换行符，已读取所有文件内容 */ 
			printf("\033[1;37;44m 文件内容已读取完毕，客户端共发送消息：%d 条\033[0m \n ", message_num); 
			break;
			/* 文件内容内容读取完毕 */
		}
		if(size > 1){							 /*读到数据*/		
			write(s, buf, strlen(buf)); 		/*发送给服务器*/
			printf("\033[1;37;46m  客户端已发送数据:\033[0m \033[1;37;45m%.*s \033[0m \n",size-1,buf);
			size = read(s, buffer, 1024);		/*从服务器读取数据,存入buffer*/
			if (size == 0) {  /* 服务器无返回结果，可判断服务器关闭*/ 
				printf("\033[1;37;44m 服务器中断，客户端共发送消息：%d 条\033[0m \n ", message_num); 
				printf("\033[1;37;41m 服务器已关闭, 无接收信息，客户端将退出\033[0m \n ");
				break;
			}
			printf("\033[1;37;43m 已接收服务器数据:\033[0m \033[1;37;40m%.*s \033[0m \n \n",strlen(buffer)-1,buffer); /*写到标准输出*/
			message_num += 1;		
		}
	}
	close(fd);	
}

void notice() {
	printf("\033[1;37;41m Ctrl+C 服务器中断关闭 \033[0m \n ");
	exit(0);
}

/*服务器对客户端的处理*/
void process_conn_server(int s)
{
	ssize_t size = 0;
	char buffer[1024]={0};							/*数据的缓冲区*/
	fa = fopen("./server.txt", "w");  /* 新建记录文件 */
	fclose(fa);
	for(;;){									/*循环处理过程*/	
		signal(SIGINT, notice);	
		size = read(s, buffer, 1024);			/*从套接字中读取数据放到缓冲区buffer中*/
		if(size == 0){							/*没有数据*/
			return;	
		}
		/*构建响应字符，为接收到客户端字节的数量*/
		/* 将客户端请求的信息写入文件 */ 
		fa = fopen("./server.txt", "a");
		fprintf(fa, "\nReceived: ");
		fprintf(fa,buffer);  /* 客户端发送的消息 */ 
		fprintf(fa, "Reply: %d bytes altogether\n", size);  /* 客户端消息的字节 */
        fclose(fa);  /* 关闭写入*/
		sprintf(buffer, "%d bytes altogether\n", size);  /* 将返回客户端消息，放入缓冲区buffer */
		write(s, buffer, strlen(buffer) + 1);/*将buffer发给客户端*/
		memset(buffer, 0, sizeof(buffer)); /* 清空buffer */  
	}	

}
