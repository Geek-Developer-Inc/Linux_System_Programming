#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h> /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#define TCP_SERVER_PORT        8888                 /*  �˿ں�  */
#define TCP_SERVER_IP          INADDR_ANY           /*  IP��ַ  */
#define TCP_SERVER_BACKLOG     10                   /*  5/10/128  */
#define TCP_SERVER_BUFF_SIZE   128                  /*  ���ݻ�������С  */

struct thread_arg
{
	int clifd;
	struct sockaddr_in cliaddr;
};

void* start_routine(void* arg)
{
	char recvmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  �������˽������ݻ�����  */
	char sendmsg[TCP_SERVER_BUFF_SIZE] = { '\0' };  /*  �������˷������ݻ�����  */

	struct thread_arg *sarg= (struct thread_arg*)arg;

	ssize_t reclen;
	memset(&reclen, 0, sizeof(reclen));

	while (1)
	{
		bzero(recvmsg, TCP_SERVER_BUFF_SIZE);
		reclen = recv(sarg->clifd, recvmsg, TCP_SERVER_BUFF_SIZE, 0);
		if (reclen > 0)
		{
			recvmsg[reclen] = '\0';
			printf("[TCP Server]: From %s client message: (%s)\n", inet_ntoa(sarg->cliaddr.sin_addr), recvmsg);

			strcpy(sendmsg, recvmsg);
			send(sarg->clifd, sendmsg, strlen(sendmsg) + 1, 0);
			printf("[TCP Server]: To client send message: (%s)\n", inet_ntoa(sarg->cliaddr.sin_addr));
		}
		else
		{
			break;
		}
	}
	close(sarg->clifd);
	printf("TCP Server close\n");
	return NULL;
}

/*  һ����һ�߳�  */
int tcp_server_thread(void)
{
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

	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	socklen_t cliaddrlen = sizeof(cliaddr);
	int clifd = 0;
	struct thread_arg arg;
	pthread_t thread = { 0 };

	printf("[TCP Server]: Waiting for client's link request\n");
	while (1)
	{
		/*  �ȴ����Կͻ��˵���������  */
		memset(&cliaddr, 0, sizeof(cliaddr));
		cliaddrlen = sizeof(cliaddr);
		clifd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddrlen);
		if (-1 == clifd)
		{
			printf("[TCP Server]: Accept a connection on a clifd error\n");
			return -4;
		}
		printf("[TCP Server]: Accept a connection on a clifd succeed\n");

		memset(&arg, 0, sizeof(arg));
		arg.clifd = clifd;
		arg.cliaddr.sin_addr = cliaddr.sin_addr;
		pthread_create(&thread, NULL, start_routine, (void*)&arg);
	}
	close(sockfd);
	printf("[TCP Server]: Close\n");
	return 0;
}

int main(void)
{
	tcp_server_thread();
	return 0;
}

/*
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
				  void *(*start_routine) (void *), void *arg);

Compile and link with -pthread.

*/