#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/select.h>

#define TCP_SERVER_PORT        8888                 /*  端口号  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP地址  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  数据缓冲区大小  */

#define STANDARD_SELECT_FLAG   1                    /*  select 的标准用法和普通用法，0 普通用法，1 标准用法  */

#if (STANDARD_SELECT_FLAG == 0)
/*  TCP Server I/O 多路复用 select  */
static int tcp_server_select(void)
{
	char recvmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  服务器端接收数据缓冲区  */
	char sendmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  服务器端发送数据缓冲区  */

	/*  创建套接字 sockfd  */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("[TCP Server]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[TCP Server]: Create a socket file descriptor succeed\n");

	/*  填充数据结构体  struct sockaddr_in  */
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;                   /*  地址族：IPv4  */
	seraddr.sin_port = htons(TCP_SERVER_PORT);      /*  端口号  */
	seraddr.sin_addr.s_addr = htonl(TCP_SERVER_IP); /*  IP地址  */

	/*  绑定端口号和IP地址到套接字 sockfd  */
	if (-1 == bind(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr)))
	{
		printf("[TCP Server]: Bind a server IP addr  and port to sockfd error\n");
		return -2;
	}
	printf("[TCP Server]: Bind a server IP addr  and port to sockfd succeed\n");

	/*  开始监听，设置最大链接数  */
	if (-1 == listen(sockfd, TCP_SERVER_BACKLOG))
	{
		printf("[TCP Server]: Listen for connections on a sockfd error\n");
		return -3;
	}
	printf("[TCP Server]: Listen for connections on a sockfd succeed\n");

	fd_set rfds = { 0 };
	fd_set rset = { 0 };
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	int maxfds = sockfd;

	printf("[TCP Server]: Waiting for client's link request\n");

	while (1)
	{
		rset = rfds;

		/*  等待来自客户端的连接请求  */
		struct sockaddr_in cliaddr;

		int nready = select(maxfds + 1, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset))
		{
			memset(&cliaddr, 0, sizeof(cliaddr));
			socklen_t cliaddrlen = sizeof(cliaddr);
			int clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
			if (-1 == clifd)
			{
				printf("[TCP Server]: Accept a connection on a clifd error\n");
				return -4;
			}
			printf("[TCP Server]: Accept a connection on a clifd succeed\n");
			
			FD_SET(clifd, &rfds);

			if (clifd > maxfds)
			{
				maxfds = clifd;
			}

			if (0 == --nready)
			{
				continue;
			}
		}
		for (int i = sockfd + 1; i <= maxfds; i++)
		{
			if (FD_ISSET(i, &rset))
			{
				bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
				ssize_t reclen = recv(i, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
				if (reclen > 0)
				{
					recvmsg[reclen] = '\0';
					printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(cliaddr.sin_addr), recvmsg);
					bzero(sendmsg, TCP_SERVER_BUFF_SIZE);
					strcpy(sendmsg, recvmsg);
					send(i, sendmsg, strlen(sendmsg) + 1, 0);
					printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(cliaddr.sin_addr));
					if (0 == strncmp("exit", recvmsg, 4))
					{
						break;
					}
				}
				else if (0 == reclen)
				{
					FD_CLR(i, &rfds);
					printf("disconnect\n");
					close(i);
				}
				if (0 == --nready)
				{
					break;
				}
			}
		}
	}
	close(sockfd);
	printf("[TCP Server]: Close\n");
	return 0;
}
#else
/*  TCP Server I/O 多路复用 select  */
static int tcp_server_select(void)
{
	char recvmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  服务器端接收数据缓冲区  */
	char sendmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  服务器端发送数据缓冲区  */

	/*  创建套接字 sockfd  */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("[TCP Server]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[TCP Server]: Create a socket file descriptor succeed\n");

	/*  填充数据结构体  struct sockaddr_in  */
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;                   /*  地址族：IPv4  */
	seraddr.sin_port = htons(TCP_SERVER_PORT);      /*  端口号  */
	seraddr.sin_addr.s_addr = htonl(TCP_SERVER_IP); /*  IP地址  */

	/*  绑定端口号和IP地址到套接字 sockfd  */
	if (-1 == bind(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr)))
	{
		printf("[TCP Server]: Bind a server IP addr  and port to sockfd error\n");
		return -2;
	}
	printf("[TCP Server]: Bind a server IP addr  and port to sockfd succeed\n");

	/*  开始监听，设置最大链接数  */
	if (-1 == listen(sockfd, TCP_SERVER_BACKLOG))
	{
		printf("[TCP Server]: Listen for connections on a sockfd error\n");
		return -3;
	}
	printf("[TCP Server]: Listen for connections on a sockfd succeed\n");

	fd_set rfds = { 0 };
	fd_set rset = { 0 };
	fd_set wfds = { 0 };
	fd_set wset = { 0 };
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(sockfd, &rfds);

	int maxfds = sockfd;

	printf("[TCP Server]: Waiting for client's link request\n");

	while (1)
	{
		rset = rfds;
		wset = wfds;
		/*  等待来自客户端的连接请求  */
		struct sockaddr_in cliaddr;

		int nready = select(maxfds + 1, &rset, &wset, NULL, NULL);
		if (FD_ISSET(sockfd, &rset))
		{
			memset(&cliaddr, 0, sizeof(cliaddr));
			socklen_t cliaddrlen = sizeof(cliaddr);
			int clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
			if (-1 == clifd)
			{
				printf("[TCP Server]: Accept a connection on a clifd error\n");
				return -4;
			}
			printf("[TCP Server]: Accept a connection on a clifd succeed\n");

			FD_SET(clifd, &rfds);

			if (clifd > maxfds)
			{
				maxfds = clifd;
			}

			if (0 == --nready)
			{
				continue;
			}
		}
		for (int i = sockfd + 1; i <= maxfds; i++)
		{
			if (FD_ISSET(i, &rset))
			{
				bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
				ssize_t reclen = recv(i, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
				if (reclen > 0)
				{
					recvmsg[reclen] = '\0';
					printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(cliaddr.sin_addr), recvmsg);

					FD_CLR(i, &rfds);
					FD_SET(i, &wfds);
				}
				else if (0 == reclen)
				{
					FD_CLR(i, &rfds);
					printf("disconnect\n");
					close(i);
				}
				if (0 == --nready)
				{
					break;
				}
			}
			if (FD_ISSET(i, &wset))
			{
				bzero(sendmsg, TCP_SERVER_BUFF_SIZE);
				strcpy(sendmsg, recvmsg);

				send(i, sendmsg, strlen(sendmsg) + 1, 0);

				FD_CLR(i, &wfds);    /*  清除写事件标志  */
				FD_SET(i, &rfds);    /*  修改为读事件  */

				printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(cliaddr.sin_addr));
				if (0 == strncmp("exit", recvmsg, 4))
				{
					break;
				}
			}
		}
	}
	close(sockfd);
	printf("[TCP Server]: Close\n");
	return 0;
}
#endif

int main(void)
{
	tcp_server_select();
	return 0;
}

/*
#include <sys/select.h>
int select(int nfds, fd_set *readfds, fd_set *writefds,
				  fd_set *exceptfds, struct timeval *timeout);

void FD_CLR(int fd, fd_set *set);
int  FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(fd_set *set);

*/
