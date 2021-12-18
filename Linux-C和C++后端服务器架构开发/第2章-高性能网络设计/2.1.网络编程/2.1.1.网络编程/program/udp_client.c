#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_CLIENT_PORT        8080                         /*  �˿ں�  */
#define UDP_CLIENT_IP          "10.32.129.145"              /*  IP��ַ  */
#define UDP_CLIENT_BUFF_SIZE   128                          /*  ���ݻ�������С  */

/*  UDP �ͻ��˳���  */
static int udp_client(void)
{
	char recvmsg[UDP_CLIENT_BUFF_SIZE] = { '\0' };          /*  �ͻ��˽������ݻ�����  */
	char sendmsg[UDP_CLIENT_BUFF_SIZE] = { '\0' };          /*  �ͻ��˷������ݻ�����  */

	/*  �����׽��� sockfd��AF_INET��IPv4��SOCK_DGRAM��UDP  */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		printf("[UDP Client]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[UDP Client]: Create a socket file descriptor succeed\n");

	/*  ������ݽṹ��  struct sockaddr_in  */
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;                           /*  ��ַ�壺IPv4  */
	cliaddr.sin_port = htons(UDP_CLIENT_PORT);              /*  �˿ںţ����� -> ���磬���޷��Ŷ����͵������˿���ֵת���������ֽ���  */
	cliaddr.sin_addr.s_addr = inet_addr(UDP_CLIENT_IP);     /*  IP��ַ��inet_addr ������ IP ��ַת���� 32 λ�Ķ����������ֽ����ַ  */

	/*  ���������������  */
	if (-1 == connect(sockfd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)))
	{
		printf("[UDP Client]: Connection or binding error\n");
		return -2;
	}
	printf("[UDP Client]: Connection or binding succeed\n");

	ssize_t sendlen;
	ssize_t recvlen;
	socklen_t serlen;
	memset(&sendlen, 0, sizeof(ssize_t));
	memset(&recvlen, 0, sizeof(ssize_t));
	memset(&serlen, 0, sizeof(serlen));
	serlen = sizeof(cliaddr);
	strcpy(sendmsg, "Hello Server,I'm client");
	while (1)
	{
		sendlen = sendto(sockfd, sendmsg, strlen(sendmsg) + 1, 0, (struct sockaddr*)&cliaddr, serlen);
		if (sendlen > 0)
		{
			printf("[UDP Client]: Send to server message: (%s)\n", sendmsg);
			bzero(recvmsg, UDP_CLIENT_BUFF_SIZE);
			recvlen = recvfrom(sockfd, recvmsg, UDP_CLIENT_BUFF_SIZE, 0, (struct sockaddr*)&cliaddr, &serlen);
			if (recvlen > 0)
			{
				recvmsg[recvlen] = '\0';
				printf("[UDP Client]: Recv from server message: (%s)\n", recvmsg);
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
	printf("[UDP Client]: Close\n");
	return 0;
}

int main(void)
{
	udp_client();
	return 0;
}