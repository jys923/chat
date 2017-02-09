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
//#include <stdio.h>		// 표준 입출력
//#include <winsock2.h>	// 윈속 버전 2
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
//	if (argc == 2)  // 서버가 포트 번호로만 호출될 경우
//	{
//		hname = NULL;
//		sname = argv[1];
//	}
//	else // 서버가 주소와 포트 번호로 호출될 경우
//	{
//		hname = argv[1];
//		sname = argv[2];
//	}
//
//	// 소켓 생성
//	s = tcp_server(hname, sname);
//
//	// fd_set을 모두 0으로 초기화
//	FD_ZERO(&allreads);
//
//	// listening 하고 있는 socket descriptor를 감지
//	FD_SET(s, &allreads);
//
//	while (true)
//	{
//		printf("Waiting for clinet...\n");
//
//		// select 함수 종료 때마다 readmask가 다른 값을 갖게 되므로 초기화
//		readmask = allreads;
//
//		// descriptor 중 감시된 것이 있으면 함수 종료 및 반환
//		rc = select(peerlen[client_cnt] + 1, &readmask, NULL, NULL, NULL);
//
//		// 에러 처리
//		if (rc <= 0)
//			printf("Bad select return (%d)\n", rc);
//		if (rc == 0)
//			perror("All clients have been disconnected\n");
//
//		// 새로운 클라이언트가 감지되었다면
//		if (FD_ISSET(s, &readmask))
//		{
//			peerlen[client_cnt] = sizeof(peer[client_cnt]);
//
//			// 서버가 클라이언트로부터 연결요청을 받아 통신을 위한 새로운 소켓을 생성하여 연결 완성
//			peer_socket[client_cnt] = accept(s, (struct sockaddr *)&peer[client_cnt], &peerlen[client_cnt]);
//
//			// 최대수용 가능한 클라이언트 수
//			if (client_cnt >= NUM_OF_CLIENT)
//			{
//				// 서버에 먼저 출력하고
//				perror("No more clinets can be accepted!\n");
//
//				// 클라이언트에 메시지를 보냄
//				send(peer_socket[client_cnt], "No more clinets can be accepted!", 30, 0);
//
//				// 연결 종료
//				shutdown(peer_socket[client_cnt], 1);
//			}
//			else
//			{
//				// accept 함수가 실패하였다면
//				if (peer_socket[client_cnt] < 0)
//				{
//					perror("accept call failed\n");
//					exit(EXIT_FAILURE);
//				}
//
//				// 연결되었다고 알림
//				printf("\"%s\" has been connected.\n", inet_ntoa(peer[client_cnt].sin_addr));
//
//				// 새로운 socket descriptor 를 fd_set에 등록
//				FD_SET(peer_socket[client_cnt], &allreads);
//
//				// 클라이언트 counter를 증가
//				client_cnt++;
//			}
//		}
//
//		// 각 client 마다 감지된 것이 있는지 확인
//		for (i = 0; i < client_cnt; i++)
//		{
//			if (FD_ISSET(peer_socket[i], &readmask))
//			{
//				// 버퍼 초기화
//				memset(buf, 0x00, sizeof(buf));
//
//				// data를 수신함
//				rc = recv(peer_socket[i], buf, sizeof(buf), 0);
//				if (rc < 0)
//				{
//					perror("recv failed\n");
//					exit(EXIT_FAILURE);
//					break;
//				}
//
//				// quit 명령어 또는 data가 없을 때 종료
//				else if (rc == 0) // 종료
//				{
//					printf("\"%s\" has been disconnected.\n", inet_ntoa(peer[i].sin_addr));
//
//					// 앞으로 감지하지 않음
//					FD_CLR(peer_socket[i], &allreads);
//
//					// 연결 종료
//					if (shutdown(peer_socket[i], 1))
//						perror("Shutdown failed.\n");
//
//					// 마지막 인덱스의 socket descriptor를 비어있는 곳으로 옮김
//					peer_socket[i] = peer_socket[client_cnt--];
//				}
//				else // 정상적으로 수신 
//				{
//					// 수신 받은 data를 출력
//					//printf("%s\n", buf);
//
//					// recv 받은 client를 제외한 나머지 모든 client 에게 전송
//					for (j = 0; j < client_cnt; j++)
//						if (i != j)
//							send(peer_socket[j], buf, sizeof(buf), 0);
//				}
//			}
//		}
//	}
//
//	// Winsock 종료
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