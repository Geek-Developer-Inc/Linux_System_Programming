#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#define TCP_SERVER_PORT        8888                 /*  端口号  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP地址  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  数据缓冲区大小  */

#define EPOLL_EVENTS_SIZE      1024

/*  TCP Server epoll  */
static int tcp_server_epoll(void)
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

	int epfd = epoll_create(1); /*  参数只要大于0即可，实际无意义  */

	struct epoll_event events[EPOLL_EVENTS_SIZE] = { 0 };
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	socklen_t cliaddrlen = sizeof(cliaddr);
	printf("[TCP Server]: Waiting for client's link request\n");

	int nready = 0;
	int i = 0;
	int evfd = 0;
	int clifd = 0;
	ssize_t reclen;
	memset(&reclen, 0, sizeof(reclen));

	while (1)
	{
		nready = epoll_wait(epfd, events, EPOLL_EVENTS_SIZE, 5);
		if (-1 == nready)
		{
			continue;
		}

		for (i = 0; i < nready; i++)
		{
			evfd = events[i].data.fd;
			if (evfd == sockfd)
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

				ev.events = EPOLLIN;
				ev.data.fd = clifd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clifd, &ev);
			}
			else
			{
				if (events[i].events & EPOLLIN)
				{
					bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
					reclen = recv(evfd, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
					if (reclen > 0)
					{
						recvmsg[reclen] = '\0';
						printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(cliaddr.sin_addr), recvmsg);
						bzero(sendmsg, TCP_SERVER_BUFF_SIZE);
						strcpy(sendmsg, recvmsg);
						send(evfd, sendmsg, strlen(sendmsg) + 1, 0);
						printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(cliaddr.sin_addr));
					}
					else if (0 == reclen)
					{
						ev.events = EPOLLIN;
						ev.data.fd = evfd;
						epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
						printf("Client close\n");
						close(evfd);
					}
				}
			}
		}
	}
	close(sockfd);
	printf("TCP Server close\n");
	return 0;
}

int main(void)
{
	tcp_server_epoll();
	return 0;
}


#if 0

#include <sys/epoll.h>

epoll_create()
epoll_ctl()
int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
epoll_wait()
int epoll_wait(int epfd, struct epoll_event* events,
	int maxevents, int timeout);

#define MAX_EVENTS 10
struct epoll_event ev, events[MAX_EVENTS];
int listen_sock, conn_sock, nfds, epollfd;

/* Code to set up listening socket, 'listen_sock',
   (socket(), bind(), listen()) omitted */

epollfd = epoll_create1(0);
if (epollfd == -1) {
	perror("epoll_create1");
	exit(EXIT_FAILURE);
}

ev.events = EPOLLIN;
ev.data.fd = listen_sock;
if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
	perror("epoll_ctl: listen_sock");
	exit(EXIT_FAILURE);
}

for (;;) {
	nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
	if (nfds == -1) {
		perror("epoll_wait");
		exit(EXIT_FAILURE);
	}

	for (n = 0; n < nfds; ++n) {
		if (events[n].data.fd == listen_sock) {
			conn_sock = accept(listen_sock,
				(struct sockaddr*)&addr, &addrlen);
			if (conn_sock == -1) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			setnonblocking(conn_sock);
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = conn_sock;
			if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
				&ev) == -1) {
				perror("epoll_ctl: conn_sock");
				exit(EXIT_FAILURE);
					   }
				   }
		else {
			do_use_fd(events[n].data.fd);
		}
			   }
		   }


#endif