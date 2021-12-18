#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <poll.h>

#define TCP_SERVER_PORT        8888                 /*  端口号  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP地址  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  数据缓冲区大小  */

#define POLLFD_SIZE            256

/*  TCP Server poll  */
static int tcp_server_poll(void)
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

	struct pollfd fds[POLLFD_SIZE] = { 0 };
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;

	int maxfds = sockfd;
	int nready = 0;
	int clifd = 0;
	int i = 0;
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	socklen_t cliaddrlen = sizeof(cliaddr);
	ssize_t reclen;
	memset(&reclen, 0, sizeof(reclen));
	printf("[TCP Server]: Waiting for client's link request\n");

	while (1)
	{
		nready = poll(fds, maxfds + 1, -1);
		if (fds[0].revents & POLLIN)
		{
			/*  等待来自客户端的连接请求  */
			memset(&cliaddr, 0, sizeof(cliaddr));
			cliaddrlen = sizeof(cliaddr);
			clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
			if (-1 == clifd)
			{
				printf("[TCP Server]: Accept a connection on a clifd error\n");
				return -4;
			}
			printf("[TCP Server]: Accept a connection on a clifd succeed\n");

			fds[clifd].fd = clifd;
			fds[clifd].events = POLLIN;

			if (clifd > maxfds)
			{
				maxfds = clifd;
			}
			if (0 == --nready)
			{
				continue;

			}
		}
		for (i = sockfd + 1; i <= maxfds; i++)
		{

			if (fds[i].revents & POLLIN)
			{
				bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
				reclen = recv(i, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
				if (reclen > 0)
				{
					recvmsg[reclen] = '\0';
					printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(cliaddr.sin_addr), recvmsg);

					strcpy(sendmsg, recvmsg);
					send(i, sendmsg, strlen(sendmsg) + 1, 0);
					printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(cliaddr.sin_addr));
				}
				else if (0 == reclen)
				{

					fds[i].fd = -1;
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

int main(void)
{
	tcp_server_poll();
	return 0;
}

#if 0

#include <poll.h>
int poll(struct pollfd *fds, nfds_t nfds, int timeout);

struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};

struct timespec {
	long    tv_sec;         /* seconds */
	long    tv_nsec;        /* nanoseconds */
};

#endif
