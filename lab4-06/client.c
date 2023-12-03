#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024    // 메시지 버퍼 크기 정의
#define NICKNAME_LEN 32     // 닉네임 최대 길이 정의
#define FILE_SIZE 256 // 파일 사이즈

int sock; // 클라이언트 소켓

void *receive_message(void *socket);
void exit_routine();

int main() {
    struct sockaddr_in server_addr; // 서버 주소 구조체
    pthread_t recv_thread;          // 수신 스레드
    char nickname[NICKNAME_LEN];    // 사용자 닉네임
    char message[BUFFER_SIZE];      // 메시지 입력 버퍼
    int port = 50001;                // 포트번호

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");   // 서버의 IP 주소
    server_addr.sin_port = htons(port);                     // 서버의 포트 번호

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        return -1;
    }

    // 닉네임 입력 및 서버로 전송
    printf("닉네임을 입력하세요: ");

    fgets(nickname, NICKNAME_LEN, stdin);
    nickname[strcspn(nickname, "\n")] = 0;  // 개행 문자 제거
    fflush(stdin);

    send(sock, nickname, strlen(nickname), 0);  // 닉네임 서버로 전송


    // 수신 스레드 생성
    pthread_create(&recv_thread, NULL, receive_message, (void *)&sock);

    // 사용자가 메시지 입력 및 서버로 전송
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // 개행 문자 제거
        fflush(stdin);
        send(sock, message, strlen(message), 0);          // 메시지 서버로 전송
    }
    close(sock);    // 소켓 닫기
    return 0;
}

/* 서버로부터 메시지를 수신하는 스레드 함수 */
void *receive_message(void *socket) {
    int sock = *((int *)socket);    // 클라이언트 소켓
    char message[BUFFER_SIZE * 2];      // 메시지 저장 버퍼
    int length;

    // 서버로부터 메시지를 계속 수신
    while ((length = recv(sock, message, BUFFER_SIZE * 2, 0)) > 0) {
        message[length] = '\0';
        printf("%s\n", message);    // 수신된 메시지 출력
    }

    return NULL;
}
