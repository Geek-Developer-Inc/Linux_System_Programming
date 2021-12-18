#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_SERVER_PORT        8888                 /*  端口号  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP地址  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  数据缓冲区大小  */

/*  单客户端连接  */
static int tcp_server(void)
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

	printf("[TCP Server]: Waiting for client's link request\n");

	/*  等待来自客户端的连接请求  */
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	socklen_t cliaddrlen = sizeof(cliaddr);
	int clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
	if (-1 == clifd)
	{
		printf("[TCP Server]: Accept a connection on a clifd error\n");
		return -4;
	}
	printf("[TCP Server]: Accept a connection on a clifd succeed\n");
	ssize_t reclen;
	memset(&reclen, 0, sizeof(reclen));
	while (1)
	{
		bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
		reclen = recv(clifd, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
		if (reclen > 0)
		{
			recvmsg[reclen] = '\0';
			printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(cliaddr.sin_addr), recvmsg);

			strcpy(sendmsg, recvmsg);
			send(clifd, sendmsg, strlen(sendmsg) + 1, 0);
			printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(cliaddr.sin_addr));
		}
		else
		{
			break;
		}
	}
	close(clifd);
	close(sockfd);
	printf("[TCP Server]: Close\n");
	return 0;
}

int main(void)
{
	tcp_server();
	return 0;
}

/*
1.socket - 创建通信端点，返回一个套接字文件描述符(returns a file descriptor)
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
domain：指定网域， AF_INET IPv4， AF_INET6 IPv6
type：选择传输协议族， SOCK_STREAM TCP， SOCK_DGRAM UDP
protocol：基本废弃，直接赋0
成功返回网络套接字描述符 sockfd，失败返回 -1

#include <netinet/in.h>
struct sockaddr_in
{
	unsigned short sin_family;  地址族
	unsigned short sin_port;    端口号
	struct in_addr sin_addr;    IP地址
	char sin_zero[8];           未使用
};

2.bind - 向 sockfd 绑定端口号和IP地址
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
sockfd：为 socket 函数的返回值
addr：封装IP地址和端口号
addrlen：struct sockaddr 结构体的大小，sizeof(struct sockaddr)
成功返回 0，失败返回 -1

3.memset
#include <string.h>
void *memset(void *s, int c, size_t n);

4.listen - 监听套接字上的连接，设置允许的最大连接数
#include <sys/types.h>
#include <sys/socket.h>
int listen(int sockfd, int backlog);
sockfd：为 socket 函数的返回值
backlog：指定同时能处理的最大连接要求(即队列的最大长度)，通常为10或者5，最大为128
成功返回 0，失败返回 -1

5.accept, accept4 - 等待来自客户端的连接请求
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
sockfd：为 socket 函数的返回值
addr：用于接收客户端的IP地址和端口号
addrlen：struct sockaddr *addr 的大小
返回新的套接字描述符(通信套接字)，专门用于与建立客户端的通信，失败返回 -1

6.send - 在套接字上发送消息
#include <sys/types.h>
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
sockfd：为 accept 函数的返回值
buf：要发送的数据缓冲区
len：buf的数据长度
flags：一般赋为 0，阻塞
成功返回数据长度，失败 -1

7.recv - 从套接字接收消息
#include <sys/types.h>
#include <sys/socket.h>
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
sockfd：为 accept 函数的返回值
buf：存放接收数据的缓冲区
len：数据缓冲区大小
flags：一般赋为 0，阻塞
成功返回接收到数据的长度，失败 -1

#include <strings.h>
void bzero(void *s, size_t n);

#include <unistd.h>
int close(int fd);

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int inet_aton(const char *cp, struct in_addr *inp);
in_addr_t inet_addr(const char *cp);
in_addr_t inet_network(const char *cp);
char *inet_ntoa(struct in_addr in);
struct in_addr inet_makeaddr(in_addr_t net, in_addr_t host);
in_addr_t inet_lnaof(struct in_addr in);
in_addr_t inet_netof(struct in_addr in);

*/
