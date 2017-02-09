///***************************************************************************
//* Daegyeong Kim
//* 32062105
//* Dankook Univ.
//*
//* Nov 27, 2011
//* TCP chatting program
//* Client program
//*
//* Net_client.cpp
//***************************************************************************/
//
//#include <stdio.h>		// ǥ�� �����
//#include <winsock2.h>	// ���� ���� 2
////#include <windows.h>
//
//#define BUF_SIZE		1400
//#define NICKNAME_SIZE	30
//
//#pragma comment(lib, "ws2_32.lib") // winsock 2.2 library
//
//DWORD WINAPI input_loop(LPVOID s);
//DWORD WINAPI recv_loop(LPVOID s);
//void init();
//SOCKET tcp_client(char *hname, char *sname);
//static void set_address(char *hname, char *sname, struct sockaddr_in *sap, char *protocol);
//
//int main(int argc, char **argv)
//{
//	HANDLE hThread[2];
//	SOCKET s;
//
//	init();
//	s = tcp_client(argv[1], argv[2]);
//
//	hThread[0] = CreateThread(NULL, 0, input_loop, (LPVOID)s, 0, NULL);
//	hThread[1] = CreateThread(NULL, 0, recv_loop, (LPVOID)s, 0, NULL);
//
//	// ��� �����尡 ����Ǳ⸦ ��ٸ�
//	DWORD retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
//
//	switch (retval)
//	{
//	case WAIT_OBJECT_0: // hTread[0] ����
//		break;
//	case WAIT_OBJECT_0 + 1: // hTread[1] ����
//		break;
//	case WAIT_FAILED: // ���� �߻�
//		break;
//	}
//
//	// ���� ����
//	shutdown(s, 1);
//
//	// Winsock ����
//	WSACleanup();
//
//	printf("���α׷��� ����Ǿ����ϴ�.\n");
//
//	return 0;
//}
//
//DWORD WINAPI input_loop(LPVOID s)
//{
//	char nickname[NICKNAME_SIZE];
//	char buf[BUF_SIZE];
//	char buf2[BUF_SIZE];
//	SOCKET socket = (SOCKET)s;
//
//	memset(nickname, 0x00, NICKNAME_SIZE);
//	memset(buf, 0x00, BUF_SIZE);
//
//	printf("�г��� : ");
//	//gets(nickname);
//	fgets(nickname, BUF_SIZE, stdin);
//	nickname[strlen(nickname) - 1] = '\0';
//
//	sprintf(buf, "%s���� �����ϼ̽��ϴ�.\n", nickname);
//
//	// �г��� ����
//	if (send(socket, buf, BUF_SIZE, 0) < 0)
//	{
//		perror("Send failure.\n");
//		return 0;
//	}
//
//	while (true)
//	{
//		memset(buf, 0x00, BUF_SIZE);
//		memset(buf2, 0x00, BUF_SIZE);
//		printf(">> ");
//
//		// �޽��� �Է�
//		fgets(buf, BUF_SIZE, stdin);
//
//		// quit ��ɾ�� ����
//		if (!strcmp(buf, "quit\n"))
//		{
//			sprintf(buf2, "%s���� �����ϼ̽��ϴ�.\n", nickname);
//			// �г��� ����
//			if (send(socket, buf2, BUF_SIZE, 0) < 0)
//				perror("Send failure.\n");
//
//			// EOF
//			if (send(socket, 0, 0, 0) < 0)
//				perror("Send failure.\n");
//
//			break;
//		}
//		else
//		{
//			sprintf(buf2, "[%s] %s", nickname, buf);
//
//			// �޽��� ����
//			if (send(socket, buf2, BUF_SIZE, 0) < 0)
//			{
//				perror("Send failure.\n");
//				break;
//
//			}
//		}
//	}
//
//	return 0;
//}
//
//DWORD WINAPI recv_loop(LPVOID s)
//{
//
//	int rc;
//	char buf3[BUF_SIZE];
//
//	while (true)
//	{
//		memset(buf3, 0x00, BUF_SIZE);
//
//		// data�� ������
//		rc = recv((SOCKET)s, buf3, sizeof(buf3), 0);
//		if (rc < 0)
//		{
//			perror("Recv() returned error.\n");
//			break;
//		}
//		else if (rc == 0)
//		{
//			perror("Server has been disconnected\n");
//			break;
//		}
//		else
//			printf("%s>> ", buf3);
//	}
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
//SOCKET tcp_client(char *hname, char *sname)
//{
//	struct sockaddr_in peer;
//	SOCKET s;
//	int rc;
//	set_address(hname, sname, &peer, "tcp");
//
//	// ���� ���� �� TCP �������� �ڿ� �Ҵ�
//	// ����1: TCP/IP, ����2: TCP, ����3: TCP, UDP �� ��� 0
//	s = socket(PF_INET, SOCK_STREAM, 0);
//	if (s < 0)
//	{
//		perror("socket call failed");
//		exit(EXIT_FAILURE);
//	}
//
//	// Ŭ���̾�Ʈ�� Ư�� ������ ����� ���� ���� ��û
//	// �����ϸ� 0��, �����ϸ� 0�ƴ� ������(-1) ��ȯ
//	// ����1: socket descriptor, ����2: sockaddr ����ü ������ ������ ���� �ּ�
//	// ����3: ���� �ּ��� ����
//	rc = connect(s, (struct sockaddr*)&peer, sizeof(peer));
//	if (rc)
//	{
//		perror("connect call failed");
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