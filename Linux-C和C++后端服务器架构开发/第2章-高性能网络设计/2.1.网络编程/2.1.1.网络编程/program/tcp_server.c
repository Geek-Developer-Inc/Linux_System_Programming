#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_SERVER_PORT        8888                 /*  �˿ں�  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP��ַ  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  ���ݻ�������С  */

/*  ���ͻ�������  */
static int tcp_server(void)
{
	char recvmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  �������˽������ݻ�����  */
	char sendmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  �������˷������ݻ�����  */

	/*  �����׽��� sockfd  */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("[TCP Server]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[TCP Server]: Create a socket file descriptor succeed\n");

	/*  ������ݽṹ��  struct sockaddr_in  */
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;                   /*  ��ַ�壺IPv4  */
	seraddr.sin_port = htons(TCP_SERVER_PORT);      /*  �˿ں�  */
	seraddr.sin_addr.s_addr = htonl(TCP_SERVER_IP); /*  IP��ַ  */

	/*  �󶨶˿ںź�IP��ַ���׽��� sockfd  */
	if (-1 == bind(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr)))
	{
		printf("[TCP Server]: Bind a server IP addr  and port to sockfd error\n");
		return -2;
	}
	printf("[TCP Server]: Bind a server IP addr  and port to sockfd succeed\n");

	/*  ��ʼ�������������������  */
	if (-1 == listen(sockfd, TCP_SERVER_BACKLOG))
	{
		printf("[TCP Server]: Listen for connections on a sockfd error\n");
		return -3;
	}
	printf("[TCP Server]: Listen for connections on a sockfd succeed\n");

	printf("[TCP Server]: Waiting for client's link request\n");

	/*  �ȴ����Կͻ��˵���������  */
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
1.socket - ����ͨ�Ŷ˵㣬����һ���׽����ļ�������(returns a file descriptor)
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
domain��ָ������ AF_INET IPv4�� AF_INET6 IPv6
type��ѡ����Э���壬 SOCK_STREAM TCP�� SOCK_DGRAM UDP
protocol������������ֱ�Ӹ�0
�ɹ����������׽��������� sockfd��ʧ�ܷ��� -1

#include <netinet/in.h>
struct sockaddr_in
{
	unsigned short sin_family;  ��ַ��
	unsigned short sin_port;    �˿ں�
	struct in_addr sin_addr;    IP��ַ
	char sin_zero[8];           δʹ��
};

2.bind - �� sockfd �󶨶˿ںź�IP��ַ
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
sockfd��Ϊ socket �����ķ���ֵ
addr����װIP��ַ�Ͷ˿ں�
addrlen��struct sockaddr �ṹ��Ĵ�С��sizeof(struct sockaddr)
�ɹ����� 0��ʧ�ܷ��� -1

3.memset
#include <string.h>
void *memset(void *s, int c, size_t n);

4.listen - �����׽����ϵ����ӣ�������������������
#include <sys/types.h>
#include <sys/socket.h>
int listen(int sockfd, int backlog);
sockfd��Ϊ socket �����ķ���ֵ
backlog��ָ��ͬʱ�ܴ�����������Ҫ��(�����е���󳤶�)��ͨ��Ϊ10����5�����Ϊ128
�ɹ����� 0��ʧ�ܷ��� -1

5.accept, accept4 - �ȴ����Կͻ��˵���������
#include <sys/types.h>
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
sockfd��Ϊ socket �����ķ���ֵ
addr�����ڽ��տͻ��˵�IP��ַ�Ͷ˿ں�
addrlen��struct sockaddr *addr �Ĵ�С
�����µ��׽���������(ͨ���׽���)��ר�������뽨���ͻ��˵�ͨ�ţ�ʧ�ܷ��� -1

6.send - ���׽����Ϸ�����Ϣ
#include <sys/types.h>
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
sockfd��Ϊ accept �����ķ���ֵ
buf��Ҫ���͵����ݻ�����
len��buf�����ݳ���
flags��һ�㸳Ϊ 0������
�ɹ��������ݳ��ȣ�ʧ�� -1

7.recv - ���׽��ֽ�����Ϣ
#include <sys/types.h>
#include <sys/socket.h>
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
sockfd��Ϊ accept �����ķ���ֵ
buf����Ž������ݵĻ�����
len�����ݻ�������С
flags��һ�㸳Ϊ 0������
�ɹ����ؽ��յ����ݵĳ��ȣ�ʧ�� -1

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
