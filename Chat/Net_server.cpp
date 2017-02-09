///***************************************************************************
//* Daegyeong Kim
//* 32062105
//* Dankook Univ.
//*
//* Nov 27, 2011
//* TCP chatting program
//* Server program
//*
//* Net_server.cpp
//***************************************************************************/
//
//#include <stdio.h>		// ǥ�� �����
//#include <winsock2.h>	// ���� ���� 2
//
//#define BUF_SIZE		1400
//#define NUM_OF_CLIENT	63
//
//#pragma comment(lib, "ws2_32.lib") // winsock 2.2 library
//
//void init();
//SOCKET tcp_server(char *hname, char *sname);
//static void set_address(char *hname, char *sname, struct sockaddr_in *sap, char *protocol);
//
//
//int main(int argc, char **argv)
//{
//	struct sockaddr_in peer[NUM_OF_CLIENT];
//	int peerlen[NUM_OF_CLIENT];
//	char *hname, *sname;
//	int client_cnt = 0, i, j, rc;
//
//	SOCKET peer_socket[NUM_OF_CLIENT];
//	SOCKET s;
//	fd_set readmask, allreads;
//
//	char buf[BUF_SIZE];
//	const int on = 1;
//
//	memset(buf, 0x00, sizeof(buf));
//
//	init();
//
//	if (argc == 2)  // ������ ��Ʈ ��ȣ�θ� ȣ��� ���
//	{
//		hname = NULL;
//		sname = argv[1];
//	}
//	else // ������ �ּҿ� ��Ʈ ��ȣ�� ȣ��� ���
//	{
//		hname = argv[1];
//		sname = argv[2];
//	}
//
//	// ���� ����
//	s = tcp_server(hname, sname);
//
//	// fd_set�� ��� 0���� �ʱ�ȭ
//	FD_ZERO(&allreads);
//
//	// listening �ϰ� �ִ� socket descriptor�� ����
//	FD_SET(s, &allreads);
//
//	while (true)
//	{
//		printf("Waiting for clinet...\n");
//
//		// select �Լ� ���� ������ readmask�� �ٸ� ���� ���� �ǹǷ� �ʱ�ȭ
//		readmask = allreads;
//
//		// descriptor �� ���õ� ���� ������ �Լ� ���� �� ��ȯ
//		rc = select(peerlen[client_cnt] + 1, &readmask, NULL, NULL, NULL);
//
//		// ���� ó��
//		if (rc <= 0)
//			printf("Bad select return (%d)\n", rc);
//		if (rc == 0)
//			perror("All clients have been disconnected\n");
//
//		// ���ο� Ŭ���̾�Ʈ�� �����Ǿ��ٸ�
//		if (FD_ISSET(s, &readmask))
//		{
//			peerlen[client_cnt] = sizeof(peer[client_cnt]);
//
//			// ������ Ŭ���̾�Ʈ�κ��� �����û�� �޾� ����� ���� ���ο� ������ �����Ͽ� ���� �ϼ�
//			peer_socket[client_cnt] = accept(s, (struct sockaddr *)&peer[client_cnt], &peerlen[client_cnt]);
//
//			// �ִ���� ������ Ŭ���̾�Ʈ ��
//			if (client_cnt >= NUM_OF_CLIENT)
//			{
//				// ������ ���� ����ϰ�
//				perror("No more clinets can be accepted!\n");
//
//				// Ŭ���̾�Ʈ�� �޽����� ����
//				send(peer_socket[client_cnt], "No more clinets can be accepted!", 30, 0);
//
//				// ���� ����
//				shutdown(peer_socket[client_cnt], 1);
//			}
//			else
//			{
//				// accept �Լ��� �����Ͽ��ٸ�
//				if (peer_socket[client_cnt] < 0)
//				{
//					perror("accept call failed\n");
//					exit(EXIT_FAILURE);
//				}
//
//				// ����Ǿ��ٰ� �˸�
//				printf("\"%s\" has been connected.\n", inet_ntoa(peer[client_cnt].sin_addr));
//
//				// ���ο� socket descriptor �� fd_set�� ���
//				FD_SET(peer_socket[client_cnt], &allreads);
//
//				// Ŭ���̾�Ʈ counter�� ����
//				client_cnt++;
//			}
//		}
//
//		// �� client ���� ������ ���� �ִ��� Ȯ��
//		for (i = 0; i < client_cnt; i++)
//		{
//			if (FD_ISSET(peer_socket[i], &readmask))
//			{
//				// ���� �ʱ�ȭ
//				memset(buf, 0x00, sizeof(buf));
//
//				// data�� ������
//				rc = recv(peer_socket[i], buf, sizeof(buf), 0);
//				if (rc < 0)
//				{
//					perror("recv failed\n");
//					exit(EXIT_FAILURE);
//					break;
//				}
//
//				// quit ��ɾ� �Ǵ� data�� ���� �� ����
//				else if (rc == 0) // ����
//				{
//					printf("\"%s\" has been disconnected.\n", inet_ntoa(peer[i].sin_addr));
//
//					// ������ �������� ����
//					FD_CLR(peer_socket[i], &allreads);
//
//					// ���� ����
//					if (shutdown(peer_socket[i], 1))
//						perror("Shutdown failed.\n");
//
//					// ������ �ε����� socket descriptor�� ����ִ� ������ �ű�
//					peer_socket[i] = peer_socket[client_cnt--];
//				}
//				else // ���������� ���� 
//				{
//					// ���� ���� data�� ���
//					//printf("%s\n", buf);
//
//					// recv ���� client�� ������ ������ ��� client ���� ����
//					for (j = 0; j < client_cnt; j++)
//						if (i != j)
//							send(peer_socket[j], buf, sizeof(buf), 0);
//				}
//			}
//		}
//	}
//
//	// Winsock ����
//	WSACleanup();
//
//	return 0;
//}
//
//void init()
//{
//	WSADATA wsock;
//	if (WSAStartup(MAKEWORD(2, 2), &wsock) != 0)
//	{
//		perror("WSAStartup call failed");
//		exit(EXIT_FAILURE);
//	}
//}
//
//SOCKET tcp_server(char *hname, char *sname)
//{
//	struct sockaddr_in local;
//	SOCKET s;
//	const int on = 1;
//
//	set_address(hname, sname, &local, "tcp");
//	s = socket(AF_INET, SOCK_STREAM, 0);
//
//	if (s < 0)
//	{
//		perror("socket call failed");
//		exit(EXIT_FAILURE);
//	}
//
//	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)))
//	{
//		perror("setsockopt failed");
//		exit(EXIT_FAILURE);
//	}
//
//	if (bind(s, (struct sockaddr *)&local, sizeof(local)))
//	{
//		perror("bind failed");
//		exit(EXIT_FAILURE);
//	}
//
//	if (listen(s, 5))
//	{
//		perror("listen failed");
//		exit(EXIT_FAILURE);
//	}
//
//	return s;
//}
//
//static void set_address(char *hname, char *sname, struct sockaddr_in *sap, char *protocol)
//{
//	struct servent *sp;
//	struct hostent *hp;
//	char *endptr;
//	short port;
//
//	memset(sap, 0x00, sizeof(sap)); //sockaddr_in ������ 0���� �ʱ�ȭ 
//
//	sap->sin_family = AF_INET;// �ּҰ��� AF_INET���� ����
//
//	if (hname != NULL)// �ּҰ� NULL�� �ƴҰ��
//	{
//		if ((sap->sin_addr.s_addr = inet_addr(hname)) == INADDR_NONE)
//			// dot format ���ڿ��� 32��Ʈ ��Ʈ��ũ ����Ʈ ������ ���������� �ٲ۴�. 
//		{
//			hp = gethostbyname(hname);
//			// gethostbyname : �־��� ȣ��Ʈ name�� �����ϴ� hostentŸ���� ����ü�� ��ȯ�Ѵ�. 
//
//			if (hp == NULL)
//				printf("unknown host: %s\n", hname);
//
//			sap->sin_addr = *(struct in_addr *)hp->h_addr;
//		}
//	}
//	else //  �ּҰ� �������� ���� ���
//		sap->sin_addr.s_addr = htonl(INADDR_ANY);
//
//	port = strtol(sname, &endptr, 0);
//	// strtol :���ڿ��� long ���� ���������� �ٲپ� �ִ� �Լ�  
//
//	if (*endptr == '\0')
//		sap->sin_port = htons(port);
//	// ��Ʈ ��ȣ�� ��Ʈ��ũ ����Ʈ ������ �ٲ�
//
//	else // sname�� ������ �ٲٴ� ���� ����
//	{      // ���� �̸����� ����
//		sp = getservbyname(sname, protocol);
//		/* getservbyname  : /etc/services �� ���ǵ� protocol�� service�� ������ �Ǵ� ���ͳ� ��Ʈ�� ��ȯ, protocol�� TCP �̰ų� UDP�� */
//
//		if (sp == NULL)
//			printf("unknown service: %s\n", sname);
//
//		sap->sin_port = sp->s_port;
//	}
//}