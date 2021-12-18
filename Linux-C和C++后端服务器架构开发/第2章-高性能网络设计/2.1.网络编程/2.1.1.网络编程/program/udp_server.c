#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>  /*  ssize_t  */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UDP_SERVER_PORT        8888                     /*  �˿ں�  */
#define UDP_SERVER_IP          INADDR_ANY               /*  IP��ַ  */
#define UDP_SERVER_BUFF_SIZE   128                      /*  ���ݻ�������С  */

/*  UDP �������˳���  */
static int udp_server(void)
{
	char recvmsg[UDP_SERVER_BUFF_SIZE] = { '\0' };      /*  �������˽������ݻ�����  */
	char sendmsg[UDP_SERVER_BUFF_SIZE] = { '\0' };      /*  �������˷������ݻ�����  */

	/*  �����׽��� sockfd  */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		printf("[UDP Server]: Create a socket file descriptor error\n");
		return -1;
	}
	printf("[UDP Server]: Create a socket file descriptor succeed\n");

	/*  ������ݽṹ��  struct sockaddr_in  */
	struct sockaddr_in seraddr;
	memset(&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;                       /*  ��ַ�壺IPv4  */
	seraddr.sin_port = htons(UDP_SERVER_PORT);          /*  �˿ں�  */
	seraddr.sin_addr.s_addr = htonl(UDP_SERVER_IP);     /*  IP��ַ  */

	/*  �󶨶˿ںź�IP��ַ���׽��� sockfd  */
	if (-1 == bind(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr)))
	{
		printf("[UDP Server]: Bind a server IP addr  and port to sockfd error\n");
		return -2;
	}
	printf("[UDP Server]: Bind a server IP addr  and port to sockfd succeed\n");

	socklen_t serlen;
	ssize_t reclen;
	memset(&serlen, 0, sizeof(serlen));
	memset(&reclen, 0, sizeof(reclen));
	while (1)
	{
		bzero(recvmsg, UDP_SERVER_BUFF_SIZE);
		reclen = recvfrom(sockfd, recvmsg, UDP_SERVER_BUFF_SIZE, 0, (struct sockaddr*)&seraddr, &serlen);
		if (reclen > 0)
		{
			recvmsg[reclen] = '\0';
			printf("[UDP Server]: From %s client message: (%s)\n", inet_ntoa(seraddr.sin_addr), recvmsg);

			strcpy(sendmsg, recvmsg);
			sendto(sockfd, sendmsg, strlen(sendmsg) + 1, 0, (struct sockaddr*)&seraddr, serlen);
			printf("[UDP Server]: To client send message: (%s)\n", inet_ntoa(seraddr.sin_addr));
			reclen = 0;
		}
		else
		{
			break;
		}
	}
	close(sockfd);
	printf("[UDP Server]: Close\n");
	return 0;
}

int main(void)
{
	udp_server();
	return 0;
}