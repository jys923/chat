#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <json/json.h>
#include <signal.h>
#include <hiredis/hiredis.h>

#define BUF_SIZE 1024
#define MAX_CLNT 256

struct sockinfo {
	int clnt_sock;
	char id[20];// = {0, };
				//float percentage;
}; // 구조체 뒤에 세미콜론이 와야함

struct roominfo {
	int room_cnt;//[방번호]
				 //char members[20][20];// = {0, };[아이기디길이][방안에 사람수]
	char* members[20];//20명까
	int total_members;
}; // 구조체 뒤에 세미콜론이 와야함

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void send_msg_room(char * msg, int len, int room_cnt);   // send to all
void error_handling(char * msg);
void get_id_all(void);
void get_room_all(void);
void * get_cmd(void * arg);

int room_cnt_total = 0;
int room_id = 0;
int clnt_cnt = 0;//클라 수
int myclnt_cnt = 0;//sinfo에 클라 아이디 넣기 위해 순차 증가//처음 접속할떄 처리가 나을듯
int clnt_socks[MAX_CLNT];//클라마다 소켓 한개씩
pthread_mutex_t mutx;//뮤텍스 임계영역
struct sockinfo sinfo[1024];
struct sockinfo tsinfo[1024];//출력 조회 용
struct roominfo trinfo[1024];//출력 조회 용
struct roominfo rinfo[1024];
redisContext *c;
redisReply *reply, *reply2;
const char *hostname = "127.0.0.1";
int port = 6379;


int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);
	setbuf(stdout, NULL);
	//레디스
	unsigned int j;
	unsigned int i;
	json_object *dbgetobj, *dval2, *dobj2;
	//redisContext *c;
	//redisReply *reply;
	//const char *hostname = "127.0.0.1";
	//int port = 6379;
	//레디스
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr);
			redisFree(c);
		}
		else {
			printf("Connection error: can't allocate redis context\n");
		}
		exit(1);
	}
	//reply = redisCommand(c, "AUTH password");
	reply = redisCommand(c, "AUTH 159753");
	printf("AUTH: %s\n", reply->str);
	freeReplyObject(reply);
	//레디스 읽어서 방 구조체 채우기
	/* Let's check what we have inside the list */
	reply = (redisReply*)redisCommand(c, "LRANGE roomlist 0 -1");
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < reply->elements; j++) {
			//printf("%u) %s\n", j, reply->element[j]->str);
			//제이슨 풀어서 방구조체에 삽입
			dbgetobj = json_tokener_parse(reply->element[j]->str);
			rinfo[room_cnt_total].room_cnt = json_object_get_int(json_object_object_get(dbgetobj, "room_id"));
			dobj2 = json_object_object_get(dbgetobj, "members");
			//json_object_put(dbgetobj);
			rinfo[room_cnt_total].total_members = json_object_array_length(dobj2);
			for (i = 0; i < json_object_array_length(dobj2); i++)
			{
				dval2 = json_object_array_get_idx(dobj2, i);
				//id 한개씩 삽입 
				rinfo[room_cnt_total].members[i] = strdup(json_object_get_string(dval2));
				//json_object_put(dval2);
			}
			//json_object_put(dobj2);
			room_id++;
			room_cnt_total++;
			//제이슨 해제
		}
		//json_tokener_free(dbgetobj);
		freeReplyObject(reply);
		//json_object_put(dval2);
		//json_object_put(dobj2);
		//json_object_put(dbgetobj);
	}
	//json_object_put(dval2);
	//json_object_put(dobj2);
	//json_object_put(dbgetobj);
	//json_tokener_free(dbgetobj);
	//g_string_free(json_str, json_str->len);
	//freeReplyObject(reply);

	//레디스
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id, t_id_cmd;//스레드 변수
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	printf("server Listening port : %s \n", argv[1]);
	pthread_create(&t_id_cmd, NULL, get_cmd, NULL);
	//pthread_join(thread_t2, (void **)&status);

	pthread_mutex_init(&mutx, NULL);//뮤텍스 초기화
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);//소켓생성

	memset(&serv_adr, 0, sizeof(serv_adr));//소켓에 쓸 주소 포트등 생성
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)//주소 소켓 연결
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1)//소켓대기
		error_handling("listen() error");

	while (1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);//클라에서 요청이 오면 요청 받기

		pthread_mutex_lock(&mutx);//뮤텍스 묶기
		clnt_socks[clnt_cnt++] = clnt_sock;//소켓배열에 소켔넣
		int i = 0;
		for (i = 0; i < clnt_cnt; i++)
		{
			printf("clnt_socks[%d]=%d\n", i, clnt_socks[i]);
		}
		pthread_mutex_unlock(&mutx);//풀기

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);//스레드 생성 handle_clnt 실행
		pthread_detach(t_id);//스레드 종료기다림
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	pthread_detach(t_id_cmd);//스레드 종료기다림
	json_tokener_free(dbgetobj);
	redisFree(c);
	return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i, j;
	char msg[BUF_SIZE];
	//char status[21]="\0";
	json_object *dval, *dobj, *dataobj, *sendobj, *dbsendobj, *arrayobj;
	char * status;
	char * data;
	int use_room_cnt_in_thread = 0;
	int room_id_in_thread = 0;
	int room_cnt_send = NULL;//null
	char * sentfrom = NULL;
	char *copy_send_msg;
	//int room_cnt_thread=0;
	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)//msg 버퍼 크기만 msg받음 파일끝에서 0, 0만나면 매세지 보내기
	{
		printf("받은내용 : %s\n", msg);
		//제이슨 메모리에 올림
		dataobj = json_tokener_parse(msg);
		dval = json_object_object_get(dataobj, "status");
		status = json_object_get_string(dval);
		//dval=json_object_object_get(dataobj, "lenth");
		//lenth=json_object_get_int(dval);
		//dobj = json_object_object_get(dataobj, "data");
		//data=json_object_get_string(dobj);
		//fputs("insert ID:", stdout);
		//printf("len:%d status:%s data:%d\n", status/*,lenth,strlen(data)*/);
		//printf("status:%s\n",status/*,lenth,strlen(data)*/);

		if (!strcmp(status, "login"))
		{
			//소켓 구조체 아이디 추가
			pthread_mutex_lock(&mutx);//뮤텍스 묶기
			sinfo[clnt_cnt - 1].clnt_sock = clnt_sock;//소켓배열에 소켔넣
			dobj = json_object_object_get(dataobj, "data");
			data = json_object_get_string(dobj);
			//data[strlen(data)-1] = '\0';//엔터지우기
			strcpy(sinfo[clnt_cnt - 1].id, data);
			pthread_mutex_unlock(&mutx);//풀기
			printf("%s:login~~~~~~~~\n", sinfo[clnt_cnt - 1].id);
			//로구인 성공
			sendobj = json_object_new_object();
			json_object_object_add(sendobj, "status", json_object_new_string("succ_login"));
			json_object_object_add(sendobj, "data", dobj);//아이디
														  //strcat(json_object_to_json_string(sendobj), "\r\n");
			write(clnt_sock, strcat(json_object_to_json_string(sendobj), "\r\n"), strlen(strcat(json_object_to_json_string(sendobj), "\r\n")));
			//메모리해제
			//json_object_put(sendobj);
			//레디스에 있는 msg 보내기
			//Rpush 했으니까 Lpop
			reply = (redisReply*)redisCommand(c, "LLEN chat_list_%s", data);
			for ( i = 0; i < reply->integer; i++)
			{
				reply2 = (redisReply*)redisCommand(c, "LPOP chat_list_%s", data);
				printf("%u) %s\n", i, reply2->str);
				write(clnt_sock, strcat(reply2->str, "\r\n"), strlen(strcat(reply2->str, "\r\n")));
			}
			freeReplyObject(reply);
			freeReplyObject(reply2);
			//json_object_put(sendobj);
			//json_object_put(dataobj);
			json_object_put(dval);
			//json_tokener_free(dataobj);
			//json_object_put(status);
			//write(clnt_sock, strcat(json_object_to_json_string(sendobj), "\r\n"), strlen(strcat(json_object_to_json_string(sendobj), "\r\n")));
		}
		else if (!strcmp(status, "mk_room"))
		{
			pthread_mutex_lock(&mutx);
			use_room_cnt_in_thread = room_cnt_total;
			room_cnt_total++;//방은 0부터
			room_id_in_thread = room_id;
			room_id++;
			pthread_mutex_unlock(&mutx);
			//방만들고 멤버추가
			dobj = json_object_object_get(dataobj, "data");
			//dobj를 스트링으로
			rinfo[use_room_cnt_in_thread].total_members = json_object_array_length(dobj);


			arrayobj = json_object_new_array();
			for (i = 0; i < json_object_array_length(dobj); i++)
			{
				dval = json_object_array_get_idx(dobj, i);
				printf("%s\n", json_object_get_string(dval));
				json_object_array_add(arrayobj, json_object_new_string(json_object_get_string(dval)));
				//id 한개씩 삽입 
				rinfo[use_room_cnt_in_thread].members[i] = strdup(json_object_get_string(dval));
			}
			rinfo[use_room_cnt_in_thread].room_cnt = room_id_in_thread;

			//json_object_array_length(dobj); 토탈
			//room_id_in_thread-방아이디
			//방아이디 토탈 [ㅁ,ㅍ,ㅊ]
			//레디스에 방 삽입
			dbsendobj = json_object_new_object();
			json_object_object_add(dbsendobj, "room_id", json_object_new_int(room_id_in_thread));//방번호
			json_object_object_add(dbsendobj, "members_total", json_object_new_int(json_object_array_length(dobj)));
			json_object_object_add(dbsendobj, "members", arrayobj);
			reply = (redisReply*)redisCommand(c, "RPUSH roomlist %s", json_object_to_json_string(dbsendobj));
			freeReplyObject(reply);
			json_object_put(dbsendobj);
			json_object_put(arrayobj);
			//레디스에 방 삽입
			//room_cnt_thread++;
			//pthread_mutex_unlock(&mutx);
			//room_cnt_total++;// 전역변수 세마포어 해주기 
			//pthread_mutex_unlock(&mutx);
			//방번호 모든 클라한테 보내
			//write(clnt_socks[i], msg, len);
			//clnt_sock-내 소켓  

			sendobj = json_object_new_object();

			//json_object_object_add(sendobj, "lenth", json_object_new_int(999));
			//length 틀리게 옴 나중에
			json_object_object_add(sendobj, "status", json_object_new_string("succ_mk_room"));
			json_object_object_add(sendobj, "data", json_object_new_int(rinfo[use_room_cnt_in_thread].room_cnt));//방번호
			json_object_object_add(sendobj, "members", json_object_object_get(dataobj, "data"));
			json_object_object_add(sendobj, "sentfrom", json_object_object_get(dataobj, "sentfrom"));
			//배열 비우고 내용복사
			//msg[0]= '\0'; // 배열 내용 지움
			//strncpy(msg, json_object_to_json_string(sendobj);,sizeof(json_object_to_json_string(sendobj);));// str1의 내용을 str2길이만큼 str2로 복사

			//보낼 msg 복사해놓고 매모리 해제
			copy_send_msg = NULL;
			copy_send_msg = strdup(json_object_to_json_string(sendobj));
			// 메모리 해제
			json_object_put(sendobj);

			printf("msg=%s\n", copy_send_msg);
			printf("rinfo[room_cnt_total].room_cnt%d\n", rinfo[use_room_cnt_in_thread].room_cnt);
			//write(clnt_sock, json_object_to_json_string(sendobj), strlen(json_object_to_json_string(sendobj)));
			for (i = 0; i < room_cnt_total; i++)
			{
				if (rinfo[i].room_cnt == room_id_in_thread)
				{
					send_msg_room(copy_send_msg, strlen(copy_send_msg), i);
					break;
				}
			}
			//send_msg_room(copy_send_msg, strlen(copy_send_msg),rinfo[use_room_cnt_in_thread].room_cnt);
			for (i = 0; i < room_cnt_total; i++)
			{
				printf("rinfo[%d].room_cnt:%d\n", i, rinfo[i].room_cnt);
				for (j = 0; j < rinfo[i].total_members; j++)
				{
					printf("rinfo[%d].members[%d]:%s\n", i, j, rinfo[i].members[j]);
				}
			}
			//free(copy_send_msg);
		}
		else if (!strcmp(status, "send_msg"))
		{
			//보낼 방번호
			dobj = json_object_object_get(dataobj, "room_cnt");
			room_cnt_send = json_object_get_int(dobj);
			//보낸 사람
			dobj = json_object_object_get(dataobj, "sentfrom");
			sentfrom = json_object_get_string(dobj);
			dobj = json_object_object_get(dataobj, "data");
			data = json_object_get_string(dobj);
			//제이슨 만들기 
			sendobj = json_object_new_object();
			json_object_object_add(sendobj, "status", json_object_new_string("send_msg"));
			json_object_object_add(sendobj, "room_cnt", json_object_new_int(room_cnt_send));
			json_object_object_add(sendobj, "sentfrom", json_object_new_string(sentfrom));
			json_object_object_add(sendobj, "data", json_object_new_string(data));

			//메모리에 있는 msg 복사해 놓고
			copy_send_msg = NULL;
			copy_send_msg = strdup(json_object_to_json_string(sendobj));
			// 메모리 해제
			json_object_put(sendobj);
			//printf("msg=%s\n", msg);
			printf("sendmsg=%s\n", copy_send_msg);
			for (i = 0; i < room_cnt_total; i++)
			{
				if (rinfo[i].room_cnt == room_cnt_send)
				{
					send_msg_room(copy_send_msg, strlen(copy_send_msg), i);
					break;
				}
			}
			free(copy_send_msg);
		}
		else if (!strcmp(status, "edit_room_members"))//한번에 여러명 가능 
		{
			//추가할 방번호
			dobj = json_object_object_get(dataobj, "room_cnt");
			room_cnt_send = json_object_get_int(dobj);
			//초대한 사람
			dobj = json_object_object_get(dataobj, "sentfrom");
			sentfrom = json_object_get_string(dobj);

			//초대할 아이디들
			dobj = json_object_object_get(dataobj, "data");
			//총 방 갯수 반복으로 방정보 찾기
			for (i = 0; i < room_cnt_total; i++)
			{
				//보낼 방번호로 구조체 검색
				if (rinfo[i].room_cnt == room_cnt_send)
				{
					//초대한 아이디가 없을때 방삭제
					//쓸일 없을듯
					if (json_object_array_length(dobj) == 0)
					{
						//제이슨 만들기 
						sendobj = json_object_new_object();
						json_object_object_add(sendobj, "status", json_object_new_string("rm_room"));
						json_object_object_add(sendobj, "room_cnt", json_object_new_int(room_cnt_send));
						json_object_object_add(sendobj, "sentfrom", json_object_new_string(sentfrom));
						//json_object_object_add(sendobj, "data", json_object_new_string(data));
						copy_send_msg = strdup(json_object_to_json_string(sendobj));
						// 메모리 해제
						json_object_put(sendobj);
						send_msg_room(copy_send_msg, strlen(copy_send_msg), room_cnt_send);
						printf("edit_room_members:\n%s\n[%d]번 방 지워짐:\n", copy_send_msg, json_object_array_length(dobj));
						//fputs("방지워짐",stdout);
						pthread_mutex_lock(&mutx);
						memmove(rinfo + (i), rinfo + ((i + 1)), sizeof(rinfo) - (i));
						//총방수만 줄이고 방아이디 그대로
						room_cnt_total--;
						pthread_mutex_unlock(&mutx);
						break;
					}
					//방 멤버 수정
					else
					{
						//send_msg_room(copy_send_msg, strlen(copy_send_msg), i);
						rinfo[i].total_members = json_object_array_length(dobj);
						for (j = 0; j < json_object_array_length(dobj); j++)
						{
							dval = json_object_array_get_idx(dobj, j);
							printf("%s\n", json_object_get_string(dval));
							//id 한개씩 삽입 
							rinfo[i].members[j] = strdup(json_object_get_string(dval));
						}
						rinfo[i].room_cnt = i;

						//제이슨 만들기 
						sendobj = json_object_new_object();
						json_object_object_add(sendobj, "status", json_object_new_string("succ_edit_room_members"));
						json_object_object_add(sendobj, "room_cnt", json_object_new_int(room_cnt_send));
						json_object_object_add(sendobj, "sentfrom", json_object_new_string(sentfrom));
						//json_object_object_add(sendobj, "data", json_object_new_string(data));

						copy_send_msg = strdup(json_object_to_json_string(sendobj));
						// 메모리 해제
						json_object_put(sendobj);
						printf("edit_room_members:sendmsg=%s\n", copy_send_msg);
						//구조체에서 보낼 방번호 검색해서 찾기
						for (i = 0; i < room_cnt_total; i++)
						{
							if (rinfo[i].room_cnt == room_cnt_send)
							{
								send_msg_room(copy_send_msg, strlen(copy_send_msg), i);
								break;
							}
						}
						break;
					}
				}
			}
			//send_msg_room(copy_send_msg, strlen(copy_send_msg), rinfo[room_cnt_send].room_cnt);
			free(copy_send_msg);
		}
		else if (!strcmp(status, "rm_member_out_room"))
		{
			//보낼 방번호
			dobj = json_object_object_get(dataobj, "room_cnt");
			room_cnt_send = json_object_get_int(dobj);
			//보낸 사람
			dobj = json_object_object_get(dataobj, "sentfrom");
			sentfrom = json_object_get_string(dobj);
			//dobj = json_object_object_get(dataobj, "data");
			//data = json_object_get_string(dobj);
			printf("room_cnt_total:%d\n", room_cnt_total);

			for (i = 0; i < room_cnt_total; i++)
			{
				printf("rinfo[%d].room_cnt:%d\n", i, rinfo[i].room_cnt);
				if (rinfo[i].room_cnt == room_cnt_send)
				{
					if (rinfo[i].total_members > 1)
					{
						//멤버지우기
						for (j = 0; j < rinfo[i].total_members; j++)
						{
							if (!strcmp(rinfo[i].members[j], sentfrom))
							{
								pthread_mutex_lock(&mutx);
								//memmove(sinfo + (i), sinfo + ((i + 1)), sizeof(sinfo) - (i));
								memmove(rinfo[i].members + (j), rinfo[i].members + ((j + 1)), sizeof(rinfo[i].members) - (j));
								rinfo[i].total_members--;
								pthread_mutex_unlock(&mutx);
								//레디스에서 i+1로 리스트 수정
								break;
							}
						}
						break;
					}
					else// if(rinfo[i].total_members>=1)
					{
						//방지우기
						pthread_mutex_lock(&mutx);
						memmove(rinfo + (i), rinfo + ((i + 1)), sizeof(rinfo) - (i));
						//총방수만 줄이고 방아이디 그대로
						room_cnt_total--;
						pthread_mutex_unlock(&mutx);
						//레디스에서 i+1로 리스트 지우기
						break;
					}
				}
			}
			for (i = 0; i < room_cnt_total; i++)
			{
				printf("rinfo[%d].room_cnt:%d\n", i, rinfo[i].room_cnt);
				for (j = 0; j < rinfo[i].total_members; j++)
				{
					printf("rinfo[%d].members[%d]:%s\n", i, j, rinfo[i].members[j]);
				}
			}
		}
		else//if(!strcmp(status,"send_msg")) 
		{
			//방안 모든 멤버에게 메시지 전달
			send_msg(msg, str_len);
			send_msg(msg, sizeof(msg));
		}
		//~~~~~~~~~~~~~제이슨 메모리 해제
		//json_object_put(sendobj);
		//json_object_put(dataobj);
		//json_object_put(dval);
		//json_object_put(dobj);
		//json_tokener_free(dataobj);
	}
	pthread_mutex_lock(&mutx);
	//clnt_socks[i] 제거
	for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
	{
		//스레드 생성시 전달한 clnt_sock이랑 그러니까 스레드 자신의 소켓정보 ==모든 소켓정보가 저장된 배열 clnt_socks[] 이랑 같으면
		if (clnt_sock == clnt_socks[i])
		{
			//while(i++<clnt_cnt-1)
			//memmove 이용
			//clnt_socks[i]=clnt_socks[i+1];
			memmove(clnt_socks + (i), clnt_socks + ((i + 1)), sizeof(clnt_socks) - (i));
			//memmove(clnt_socks + (i * sizeof(int)), clnt_socks + ((i + 1) * sizeof(int)), sizeof(clnt_socks) - (i * sizeof(int)));
			break;
		}
	}
	//sinfo[i]제거
	for (i = 0; i < clnt_cnt; i++)   // remove disconnected client
	{
		if (clnt_sock == sinfo[i].clnt_sock)
		{
			memmove(sinfo + (i), sinfo + ((i + 1)), sizeof(sinfo) - (i));
			break;
		}
	}

	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void send_msg_room(char * msg, int len, int room_cnt)
// send to room msg는 이미 제이슨 가공된 형태로
{
	pthread_mutex_lock(&mutx);
	//구조체 내용 복사 해놓자
	struct roominfo rinfo_send_msg = rinfo[room_cnt];
	pthread_mutex_unlock(&mutx);
	int i, j = 0;
	int total_members = /*rinfo[room_cnt]*/rinfo_send_msg.total_members;
	int clnt_socks_for_send[20];
	//클라마다 소켓 한개씩
	//char* members_for_send[20];//20명까
	////방에 소속된 사람 아이디 members_for_send에 복사
	//for(i = 0 ;i<total_members ;i++)
	//{
	//	members_for_send[i]=rinfo[room_cnt].members[i];
	//	printf("rinfo[room_cnt].members[i]:%s\n",rinfo[room_cnt].members[i]);
	//} 
	//err	
	//printf("sizeof(sinfo):%d\n",sizeof(sinfo));
	//printf("sizeof(sinfo):num%d\n",sizeof(sinfo) / sizeof(sinfo[0]));
	//printf("sizeof(members_for_send):%d\n",sizeof(members_for_send));
	//sizeof(_Array) / sizeof(_Array[0])
	//#define _countof(_Array)     sizeof(_Array) / sizeof(_Array[0])
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr);
			redisFree(c);
		}
		else {
			printf("Connection error: can't allocate redis context\n");
		}
		exit(1);
	}
	reply = redisCommand(c, "AUTH 159753");
	printf("AUTH: %s\n", reply->str);
	freeReplyObject(reply);
	//
	for (j = 0; j < total_members; j++)
	{
		for (i = 0; i < clnt_cnt/*sizeof(sinfo) / sizeof(sinfo[0])*/; i++)
		{
			//id로 소켓번호 찾기
			if (!strcmp(/*rinfo[room_cnt]*/rinfo_send_msg.members[j], sinfo[i].id))
			{
				//소켓 배열에 넣고 증가 
				clnt_socks_for_send[j] = sinfo[i].clnt_sock;
				break;
				//close(sock);
				//exit(0);
			}
			if (i == clnt_cnt - 1)
			{
				printf("[%s]:단절중\n", /*rinfo[room_cnt]*/rinfo_send_msg.members[j]);
				//디비에 넣기
				//reply = redisCommand(c, "AUTH password");
				reply = (redisReply*)redisCommand(c, "RPUSH chat_list_%s %s", rinfo_send_msg.members[j],msg);
				freeReplyObject(reply);

			}
		}
		//id로 못찾았으면 디비에 저장
		//printf("[%s]:단절중\n", rinfo[room_cnt].members[j]);

	}


	//for (i = 0; i<clnt_cnt/*sizeof(sinfo) / sizeof(sinfo[0])*/; i++)
	//{
	//	for (j = 0; j<total_members; j++)
	//	{
	//		//id로 소켓번호 찾기
	//		if (!strcmp(rinfo[room_cnt].members[j], sinfo[i].id))
	//		{
	//			//소켓 배열에 넣고 증가 
	//			clnt_socks_for_send[j] = sinfo[i].clnt_sock;
	//			break;
	//			//close(sock);
	//			//exit(0);
	//		}
	//	}
	//	//id로 못찾았으면 디비에 저장
	//	printf();
	//}

	//int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < total_members; i++)
	{
		strncat(msg, "\r\n", 4);
		write(clnt_socks_for_send[i], msg, len);
		//write(clnt_socks_for_send[i], strcat(msg, "\r\n"), strlen(strcat(msg, "\r\n")));		
	}
	pthread_mutex_unlock(&mutx);
	/* Disconnects and frees the context */
	redisFree(c);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
/*
int room_cnt_total=0;
int room_id = 0;
int clnt_cnt=0;//클라 수
int myclnt_cnt=0;//sinfo에 클라 아이디 넣기 위해 순차 증가//처음 접속할떄 처리가 나을듯
int clnt_socks[MAX_CLNT];//클라마다 소켓 한개씩
pthread_mutex_t mutx;//뮤텍스 임계영역
struct sockinfo sinfo[1024];
struct roominfo rinfo[1024];
*/
void get_id_all(void)
{
	int i;//, j;
	printf("접속자:[%d]\n", clnt_cnt);
	for (i = 0; i < clnt_cnt; i++)
	{
		printf("sinfo[%d].id:%s , sinfo[%d].clnt_sock:%d\n", i, sinfo[i].id, i, sinfo[i].clnt_sock);
	}
}

//void get_id(char * id)
//{
//	fputs(msg, stderr);
//	fputc('\n', stderr);
//	exit(1);
//}

void get_room_all(void)
{
	int i, j;
	printf("방갯수:[%d]\n", room_cnt_total);
	for (i = 0; i < room_cnt_total; i++)
	{
		printf("rinfo[%d].room_cnt:%d\n", i, rinfo[i].room_cnt);
		for (j = 0; j < rinfo[i].total_members; j++)
		{
			printf("rinfo[%d].members[%d]:%s\n", i, j, rinfo[i].members[j]);
		}
	}
	//for (i = 0; i < room_cnt_total; i++)
	//{
	//	printf("rinfo[%d].room_cnt:%d\trinfo[%d].members:", i, rinfo[i].room_cnt, i);
	//	for (j = 0; j < rinfo[i].total_members; j++)
	//	{
	//		printf("%s,",rinfo[i].members);
	//	}
	//	printf("\n");
	//}
}

//void get_room(int room_cnt)
//{
//	fputs(msg, stderr);
//	fputc('\n', stderr);
//	exit(1);
//}

void * get_cmd(void * arg)   // send thread main
{
	//int sock=*((int*)arg);
	//char *msg;
	char msg[BUF_SIZE];
	while (1)
	{
		//msg = 0;
		fgets(msg, BUF_SIZE, stdin);
		//fputs(msg, stdout);

		if (!strcmp(msg, "ls-s\n"))
		{
			//접속자출력
			get_id_all();
		}
		else if (!strcmp(msg, "ls-r\n"))
		{
			//방 정보 출력
			get_room_all();
		}
		//else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		//{
		//	pthread_exit((void *)2);
		//	//스레드 안에서 현 스레드 종료  2를 리턴함
		//}
		else
		{
			fputs("ls-s,ls-r 명령 지원\n", stdout);
		}
		//else if(!strcmp(msg,"ls-soc\n")) 
		//{
		//}
	}
	//return (void *)(1234);
	return NULL;
}
