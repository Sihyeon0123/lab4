#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<fcntl.h>

#define BUFSIZE 65536
#define SEND_MESSAGE_BUFSIZE 1024
#define CGI_PROGRAM_PATH "./hello"

// 서버의 루트 주소를 저장
char *CURR_MY_PATH_ROOT;

void error_handling(char *);
void GET_handler(char *, char *, char *, int);
void POST_handler(char *, char *, char *, int, char*);
void CGI_handler(char *cgi_program, int client);
void request_handler(void *);

int main(int argc, char **argv){
	// 현재 디렉터리를 루트 경로로 지정
	CURR_MY_PATH_ROOT = getenv("PWD");
    int serv_sock;
    int clnt_sock;
    char message[BUFSIZE];
    int str_len;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;
	
	// 명령행 인자로 포트 번호를 받아옴
    if(argc!=2){
    	printf("Usage : %s <port>\n", argv[0]);
    	exit(1);
    }

	// 소켓 생성
    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    
    if(serv_sock == -1) {
    	error_handling("socket");
    }
    
	// 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));
    
	// 소켓에 주소 바인딩
    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1) {
    	error_handling("bind");
    }
	// 연결 요청 대기 상태로 전환
    if(listen(serv_sock, 5)==-1){
    	error_handling("listen");
    }
    

	// 클라이언트의 연결 요청을 수락하고 리퀘스트 핸들러 함수 호출
	while(1){
        clnt_addr_size=sizeof(clnt_addr);
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock==-1){
    	    error_handling("accept");
            break;
        }  
        printf("Connection Request : %s:%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        request_handler(&clnt_sock);
    }
    
    close(clnt_sock);
    return 0;
}

void request_handler(void *arg){
    char msg[BUFSIZE];
	char *firstLine[3];
	
	// 클라이언트 소켓을 정수로 형변환
	int sd = *(int*)arg;

	 // 클라이언트로부터 메시지 수신
	int rcvd = recv(sd, msg, BUFSIZE-1, 0);
	if(rcvd<=0){
		error_handling("Error about recv()!!");
	}

	printf("-----------Request message from Client-----------\n");
	printf("%s",msg);
	printf("\n-------------------------------------------------\n");
	char post_information[SEND_MESSAGE_BUFSIZE];
	char *curr_msg = NULL;
	char METHOD[4]="";
	char VERSION[10]="";
	char URL[SEND_MESSAGE_BUFSIZE]="";

	// 메시지에서 첫 번째 줄을 추출하여 파싱
	curr_msg = strtok(msg, "\n");	
	int line_count=1;


	while(curr_msg){
		if(line_count>=15) strcpy(post_information, curr_msg);
		curr_msg = strtok(NULL, "\n");
		line_count++;
	}

	firstLine[0] = strtok(msg, " \t\n");
	firstLine[1] = strtok(NULL, " \t");
	firstLine[2] = strtok(NULL, " \t\n");
	
	strcpy(METHOD, firstLine[0]);
	strcpy(URL, firstLine[1]);
	strcpy(VERSION, firstLine[2]);
	
	if(!strncmp(METHOD, "GET",3)){
		printf("GET 작동\n");
		GET_handler(VERSION, msg, URL, sd);
	}
	else if(!strncmp(METHOD, "POST",4)) {
		POST_handler(VERSION, msg, URL, sd, post_information);
		printf("POST작동\n");
		if (strstr(URL, "/cgi-bin/") != NULL) {
			printf("CGI작동\n");
            CGI_handler(CGI_PROGRAM_PATH, sd);
        }
	}
	
	// 클라이언트 소켓 종료
	shutdown(sd, SHUT_RDWR);
	close(sd);
}

void POST_handler(char *V, char *message, char *U, int client, char *PI){
	int fd, str_len;
	char FIANL_PATH[BUFSIZE];
	char VERSION[10]="";
	char URL[SEND_MESSAGE_BUFSIZE]="";
	char HTML_DATA[BUFSIZE];
	
	// 버전과 URL 파싱
	strcpy(VERSION, V);
	strcpy(URL, U);

	// 응답으로 보낼 HTML 데이터 생성 -> 여기서는 받은 값을 다시 보내준다
	sprintf(HTML_DATA, "<!DOCTYPE html><html><body><h2>%s</h2></body></html>",PI);
	
	// 잘못된 HTTP 버전일 경우 400 Bad Request 응답 전송
	if(strncmp(VERSION, "HTTP/1.0",8)!=0 && strncmp(VERSION, "HTTP/1.1",8)!=0){
		write(client, "HTTP/1.1 400 Bad Request\n",25);
	}
	else{
		// 정상적인 경우 200 OK 응답과 함께 HTML 데이터 전송
		send(client, "HTTP/1.1 200 OK\n\n",17,0);
		write(client, HTML_DATA, strlen(HTML_DATA));
	}
}

void GET_handler(char *V, char *message, char *U, int client){
	int fd, str_len;
	char SEND_DATA[SEND_MESSAGE_BUFSIZE];
	char FIANL_PATH[BUFSIZE];
	char VERSION[10]="";
	char URL[SEND_MESSAGE_BUFSIZE]="";
	
	// 버전과 URL 파싱
	strcpy(VERSION, V);
	strcpy(URL, U);

	// 잘못된 HTTP 버전일 경우 400 Bad Request 응답 전송
	if(strncmp(VERSION, "HTTP/1.0",8)!=0 && strncmp(VERSION, "HTTP/1.1",8)!=0){
		write(client, "HTTP/1.1 400 Bad Request\n",25);
	}

	// URL이 "/"인 경우 "/index.html"로 설정
	if(strlen(URL) == 1 && !strncmp(URL, "/",1)) strcpy(URL, "/index.html");

	// 파일 경로 설정
	strcpy(FIANL_PATH, CURR_MY_PATH_ROOT);
	strcat(FIANL_PATH, URL);

	if((fd=open(FIANL_PATH, O_RDONLY)) != -1){
		send(client, "HTTP/1.0 200 OK\n\n", 17, 0);
		while(1){
			str_len = read(fd, SEND_DATA, SEND_MESSAGE_BUFSIZE);
			if(str_len<=0) break;
			write(client, SEND_DATA, str_len);
		}
	}
	else {
		// 파일이 존재하지 않는 경우 404 Not Found 응답 전송
		write(client, "HTTP/1.1 404 Not Found\n", 23);
	}
}

void CGI_handler(char *cgi_program, int client) {\
    // 새로운 프로세스 생성
    FILE *fp = popen(cgi_program, "r");
    if (fp == NULL) {
        write(client, "HTTP/1.1 500 Internal Server Error\n", 35);
        return;
    }

    // CGI 프로그램의 출력을 읽어 클라이언트에 전송
    char buffer[BUFSIZE];
    size_t read_size;

    while ((read_size = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        write(client, buffer, read_size);
    }

    pclose(fp);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}