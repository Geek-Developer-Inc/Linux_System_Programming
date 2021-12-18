#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_CLIENT_PORT        8080                     /*  �˿ں�  */
#define TCP_CLIENT_IP          "10.32.129.145"          /*  IP��ַ  */
#define TCP_CLIENT_BUFF_SIZE   128                      /*  ���ݻ�������С  */

/*  TCP �ͻ��˳���  */
static int tcp_client(void)
{
	char recvmsg[TCP_CLIENT_BUFF_SIZE] = { '\0' };      /*  �ͻ��˽������ݻ�����  */
	char sendmsg[TCP_CLIENT_BUFF_SIZE] = { '\0' };      /*  �ͻ��˷������ݻ�����  */

	/*  �����׽��� sockfd  */
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd)
	{
		printf("[TCP Client]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[TCP Client]: Create a socket file descriptor succeed\n");

	/*  ������ݽṹ��  struct sockaddr_in  */
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;                       /*  ��ַ�壺IPv4  */
	cliaddr.sin_port = htons(TCP_CLIENT_PORT);          /*  �˿ںţ����� -> ���磬���޷��Ŷ����͵������˿���ֵת���������ֽ���  */
	cliaddr.sin_addr.s_addr = inet_addr(TCP_CLIENT_IP); /*  IP��ַ��inet_addr ������ IP ��ַת���� 32 λ�Ķ����������ֽ����ַ  */
    
	/*  ���������������  */
	if (-1 == connect(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)))
	{
		printf("[TCP Client]: Connection or binding error\n");
		return -2;
	}
	printf("[TCP Client]: Connection or binding succeed\n");

	ssize_t sendlen;
	ssize_t recvlen;
	memset(&sendlen, 0, sizeof(ssize_t));
	memset(&recvlen, 0, sizeof(ssize_t));
	strcpy(sendmsg, "Hello Server,I'm client");
	while (1)
	{
		sendlen = send(sockfd, sendmsg, strlen(sendmsg) + 1, 0);
		if (sendlen > 0)
		{
			printf("[TCP Client]: Send to server message: (%s)\n", sendmsg);
			bzero(recvmsg, TCP_CLIENT_BUFF_SIZE);
			recvlen = recv(sockfd, recvmsg, TCP_CLIENT_BUFF_SIZE, 0);
			if (recvlen > 0)
			{
				recvmsg[recvlen] = '\0';
				printf("[TCP Client]: Recv from server message: (%s)\n", recvmsg);
			}
			else
			{
				break;
			}
			
		}
		else
		{
			break;
		}
		sleep(1);
	}
	close(sockfd);
	printf("[TCP Client]: Close\n");
	return 0;
}

int main(void)
{
	tcp_client();
	return 0;
}