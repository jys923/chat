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
//#include <stdio.h>		// 표준 입출력
//#include <winsock2.h>	// 윈속 버전 2
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
//	// 모든 쓰레드가 종료되기를 기다림
//	DWORD retval = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
//
//	switch (retval)
//	{
//	case WAIT_OBJECT_0: // hTread[0] 종료
//		break;
//	case WAIT_OBJECT_0 + 1: // hTread[1] 종료
//		break;
//	case WAIT_FAILED: // 오류 발생
//		break;
//	}
//
//	// 연결 종료
//	shutdown(s, 1);
//
//	// Winsock 종료
//	WSACleanup();
//
//	printf("프로그램이 종료되었습니다.\n");
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
//	printf("닉네임 : ");
//	//gets(nickname);
//	fgets(nickname, BUF_SIZE, stdin);
//	nickname[strlen(nickname) - 1] = '\0';
//
//	sprintf(buf, "%s님이 입장하셨습니다.\n", nickname);
//
//	// 닉네임 전송
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
//		// 메시지 입력
//		fgets(buf, BUF_SIZE, stdin);
//
//		// quit 명령어로 종료
//		if (!strcmp(buf, "quit\n"))
//		{
//			sprintf(buf2, "%s님이 퇴장하셨습니다.\n", nickname);
//			// 닉네임 전송
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
//			// 메시지 전송
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
//		// data를 수신함
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
//	// 소켓 생성 및 TCP 프로토콜 자원 할당
//	// 인자1: TCP/IP, 인자2: TCP, 인자3: TCP, UDP 인 경우 0
//	s = socket(PF_INET, SOCK_STREAM, 0);
//	if (s < 0)
//	{
//		perror("socket call failed");
//		exit(EXIT_FAILURE);
//	}
//
//	// 클라이언트가 특정 서버와 통신을 위해 연결 요청
//	// 성공하면 0을, 실패하면 0아닌 정수값(-1) 반환
//	// 인자1: socket descriptor, 인자2: sockaddr 구조체 포인터 형태의 서버 주소
//	// 인자3: 서버 주소의 길이
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
//	memset(sap, 0x00, sizeof(sap)); //sockaddr_in 구조를 0으로 초기화 
//
//	sap->sin_family = AF_INET;// 주소값을 AF_INET으로 설정
//
//	if (hname != NULL)// 주소가 NULL이 아닐경우
//	{
//		if ((sap->sin_addr.s_addr = inet_addr(hname)) == INADDR_NONE)
//			// dot format 문자열을 32비트 네트워크 바이트 순서의 이진값으로 바꾼다. 
//		{
//			hp = gethostbyname(hname);
//			// gethostbyname : 주어진 호스트 name에 상응하는 hostent타입의 구조체를 반환한다. 
//
//			if (hp == NULL)
//				printf("unknown host: %s\n", hname);
//
//			sap->sin_addr = *(struct in_addr *)hp->h_addr;
//		}
//	}
//	else //  주소가 지정되지 않을 경우
//		sap->sin_addr.s_addr = htonl(INADDR_ANY);
//
//	port = strtol(sname, &endptr, 0);
//	// strtol :문자열을 long 형의 정수값으로 바꾸어 주는 함수  
//
//	if (*endptr == '\0')
//		sap->sin_port = htons(port);
//	// 포트 번호를 네트워크 바이트 순서로 바꿈
//
//	else // sname을 정수로 바꾸는 것이 실패
//	{      // 서비스 이름으로 가정
//		sp = getservbyname(sname, protocol);
//		/* getservbyname  : /etc/services 에 정의된 protocol의 service에 대응이 되는 인터넷 포트를 반환, protocol은 TCP 이거나 UDP임 */
//
//		if (sp == NULL)
//			printf("unknown service: %s\n", sname);
//
//		sap->sin_port = sp->s_port;
//	}
//}