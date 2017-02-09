#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
//#include <jansson.h>
#include <json/json.h>
	
#define BUF_SIZE 128
#define NAME_SIZE 20
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
int main(int argc, char *argv[])
{
	setbuf(stdout, NULL); 
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	//sprintf(name, "[%s]", argv[3]);
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);//ip
	serv_addr.sin_port=htons(atoi(argv[2]));//port
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);//보내기
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);//받기
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	//char *sArr[20] = { NULL, };    // 크기가 10인 문자열 포인터 배열을 선언하고 NULL로 초기화
	//int i = 0;                     // 문자열 포인터 배열의 인덱스로 사용할 변수
	//char *ptr;// = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름
	//char name[NAME_SIZE];
	char msg[BUF_SIZE];
	//char * name= NULL;
	char name[20] = {0, };
	int sock=*((int*)arg);
	//char name_msg[NAME_SIZE+BUF_SIZE];
	json_object *dataobj, *arrayobj;
	int room_cnt = 0;
	//json 아이디
	fputs("insert ID:", stdout);
	fgets(msg, BUF_SIZE, stdin);
	//name=msg;
	strcpy(name, msg);
	name[strlen(name) - 1] = '\0';
    
    // 메모리 할당
    //myobj = json_object_new_object();
    dataobj = json_object_new_object();
    
    json_object_object_add(dataobj, "status", json_object_new_string("login"));
    json_object_object_add(dataobj, "data", json_object_new_string(name));
    //json_object_object_add(dataobj, "data", json_object_new_string("hello"));
    //json_object_object_add(myobj, "segment", dataobj);
    
    printf("login:%s\n", json_object_to_json_string(dataobj));
    
    //json_object_put(myobj);
	write(sock, json_object_to_json_string(dataobj), strlen(json_object_to_json_string(dataobj)));
	// 메모리 해제
    json_object_put(dataobj);
	//로그인 정보 남음 막상 보내지도 안음-클라에서만 한번 보임
	char status[20] = { 0, };
	char * data = NULL;
	while(1) 
	{
		dataobj = json_object_new_object();
		fputs("insert status:", stdout);
		fgets(msg, BUF_SIZE, stdin);
		//char * statusss= NULL;

		//int mem_cnt=0;
		//char *mem[2];
		//statusss=msg;
		//memset(status, '\0', sizeof(status));
		strcpy( status, msg );
		status[strlen(status)-1] = '\0';
		if(!strcmp(status,"mk_room")) 
		{
			fputs("insert mem_id 띄어쓰기로 구분:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			//data = 0;
			data=msg;
			data[strlen(data)-1] = '\0';
			//printf("data:%s\n", data);
			char *sArr[20] = { NULL, };    // 크기가 10인 문자열 포인터 배열을 선언하고 NULL로 초기화
			int i = 0;                     // 문자열 포인터 배열의 인덱스로 사용할 변수
			//ptr = 0;
			char *ptr = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름
			//ptr = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름
			//memset(sArr, '\0', sizeof(sArr));
			
			while (ptr != NULL)            // 자른 문자열이 나오지 않을 때까지 반복
			{
				sArr[i] = ptr;             // 문자열을 자른 뒤 메모리 주소를 문자열 포인터 배열에 저장
				i++;                       // 인덱스 증가
				ptr = strtok(NULL, " ");   // 다음 문자열을 잘라서 포인터를 반환
			}
			//printf("111\n");
			//for ( i = 0; i < 20; i++)
			//{
			//	if (sArr[i] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
			//	printf("%s\n", sArr[i]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
			//}
			//printf("111\n");
			arrayobj = json_object_new_array();
			for ( i = 0; i < 20; i++)
			{
				if (sArr[i] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
				{	
					printf("%s\n", sArr[i]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
					json_object_array_add(arrayobj, json_object_new_string(sArr[i]));
				}
			}
			//printf("222\n");
			json_object_object_add(dataobj, "data", arrayobj);
			json_object_object_add(dataobj, "status", json_object_new_string(status));
			json_object_object_add(dataobj, "sentfrom", json_object_new_string(name));
			printf("mk_room:%s\n", json_object_to_json_string(dataobj));
		}else if(!strcmp(status,"send_msg")) 
		{
			room_cnt = 0;
			fputs("insert room:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			room_cnt =atoi(msg);
			//printf("good???:%d",k);
			fputs("insert data:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			data=msg;
			data[strlen(data)-1] = '\0';
			
			json_object_object_add(dataobj, "status", json_object_new_string(status));
			json_object_object_add(dataobj, "data", json_object_new_string(data));
			json_object_object_add(dataobj, "room_cnt", json_object_new_int(room_cnt));
			json_object_object_add(dataobj, "sentfrom", json_object_new_string(name));
			printf("send_msg:%s\n", json_object_to_json_string(dataobj));
		}else if (!strcmp(status, "edit_room_members"))
		{
			fputs("insert room:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			room_cnt = atoi(msg);

			fputs("insert mem_id 띄어쓰기로 구분:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			data = 0;
			data = msg;
			data[strlen(data) - 1] = '\0';
			//printf("data:%s\n", data);
			//ptr = 0;
			////char *ptr = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름
			//ptr = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름
			//memset(sArr, '\0', sizeof(sArr));
			//while (ptr != NULL)            // 자른 문자열이 나오지 않을 때까지 반복
			//{
			//	sArr[i] = ptr;             // 문자열을 자른 뒤 메모리 주소를 문자열 포인터 배열에 저장
			//	i++;                       // 인덱스 증가
			//	ptr = strtok(NULL, " ");   // 다음 문자열을 잘라서 포인터를 반환
			//}
			//for (i = 0; i < 20; i++)
			//{
			//	if (sArr[i] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
			//		printf("%s\n", sArr[i]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
			//}
			//arrayobj = json_object_new_array();
			//for (i = 0; i < 20; i++)
			//{
			//	if (sArr[i] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
			//								   //printf("%s\n", sArr[i]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
			//		json_object_array_add(arrayobj, json_object_new_string(sArr[i]));
			//}
			char *sArr1[20] = { NULL, };    // 크기가 10인 문자열 포인터 배열을 선언하고 NULL로 초기화
			int i2 = 0;                     // 문자열 포인터 배열의 인덱스로 사용할 변수

			char *ptr1 = strtok(data, " ");   // 공백 문자열을 기준으로 문자열을 자름

			while (ptr1 != NULL)            // 자른 문자열이 나오지 않을 때까지 반복
			{
				sArr1[i2] = ptr1;             // 문자열을 자른 뒤 메모리 주소를 문자열 포인터 배열에 저장
				i2++;                       // 인덱스 증가
				ptr1 = strtok(NULL, " ");   // 다음 문자열을 잘라서 포인터를 반환
			}
			for (i2 = 0; i2 < 20; i2++)
			{
				if (sArr1[i2] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
					printf("%s\n", sArr1[i2]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
			}
			arrayobj = json_object_new_array();
			for (i2 = 0; i2 < 20; i2++)
			{
				if (sArr1[i2] != NULL)           // 문자열 포인터 배열의 요소가 NULL이 아닐 때만
											   //printf("%s\n", sArr[i]);   // 문자열 포인터 배열에 인덱스로 접근하여 각 문자열 출력
					json_object_array_add(arrayobj, json_object_new_string(sArr1[i2]));
			}
			json_object_object_add(dataobj, "data", arrayobj);
			json_object_object_add(dataobj, "status", json_object_new_string(status));
			json_object_object_add(dataobj, "room_cnt", json_object_new_int(room_cnt));
			json_object_object_add(dataobj, "sentfrom", json_object_new_string(name));
			printf("edit_room_members:%s\n", json_object_to_json_string(dataobj));
		}else if (!strcmp(status, "rm_member_out_room"))
		{
			fputs("insert room:", stdout);
			fgets(msg, BUF_SIZE, stdin);
			room_cnt = atoi(msg);
			
			json_object_object_add(dataobj, "status", json_object_new_string(status));
			json_object_object_add(dataobj, "room_cnt", json_object_new_int(room_cnt));
			json_object_object_add(dataobj, "sentfrom", json_object_new_string(name));
			printf("rm_member_out_room:%s\n", json_object_to_json_string(dataobj));
		}
		
		//MK_ROOM jo,yoon,sang 
		
		//dataobj = json_object_new_object();
    
		//json_object_object_add(dataobj, "lenth", json_object_new_int(strlen(data)));
		//json_object_object_add(dataobj, "status", json_object_new_string(status));
		//json_object_object_add(dataobj, "data", json_object_new_string(data));
		
		//printf("myobj.to_string()=%s\n", json_object_to_json_string(dataobj));

		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) 
		{
			close(sock);
			exit(0);
		}
		//여기에서 프로토콜 추가 하자
		//sprintf(name_msg,"%s %s", name, msg);
		write(sock, json_object_to_json_string(dataobj), strlen(json_object_to_json_string(dataobj)));
		//write(sock, name_msg, strlen(name_msg));
		// 메모리 해제
		//arrayobj 해제 안함
		json_object_put(dataobj);
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);
		//fflush(stdout);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
