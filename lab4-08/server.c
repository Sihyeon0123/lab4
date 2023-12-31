#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 10      // 최대 클라이언트 수 정의
#define BUFFER_SIZE 1024    // 메시지 버퍼 크기 정의
#define NICKNAME_LEN 32     // 닉네임 최대 길이 정의
#define FILE_SIZE 256       // 파일 사이즈

void *handle_client(void *arg);

int clients[MAX_CLIENTS];                                   // 연결된 클라이언트 소켓 저장 배열
int n_clients = 0;                                          //  현재 연결된 클라이언트 수
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // 스레드 간 동기화를 위한 뮤텍스
pthread_mutex_t nicknameMutex = PTHREAD_MUTEX_INITIALIZER;  // 닉네임 저장을 위한 동기화

int main() {
    int server_sock, client_sock;                   // 서버 및 클라이언트 소켓
    struct sockaddr_in server_addr, client_addr;    // 소켓 주소 구조체
    pthread_t thread;                               // 스레드 ID
    socklen_t client_addr_size;                     // 클라이언트 주소 크기
    char user_input[10];
    int port = 50001;// 포트 번호

    for(int i=0; i<MAX_CLIENTS; i++){
        clients[i] = -1;
    }

    // 서버 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // 소켓에 주소 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        return -1;
    }

    // 소켓 리스닝 시작
    if (listen(server_sock, 5) == -1) {
        perror("listen");
        return -1;
    }

    printf("서버가 시작되었습니다... 다른 탭에서 클라이언트를 실행해 주세요\n");

    // 클라이언트 연결 수락 반복문
    while (1) {
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            perror("accept");
            continue;
        }

        // 클라이언트 관리를 위한 뮤텍스 잠금
        pthread_mutex_lock(&clients_mutex);
        if (n_clients < MAX_CLIENTS) {
            clients[n_clients++] = client_sock;
            // 각 클라이언트에 대한 스레드 생성 및 실행
            pthread_create(&thread, NULL, handle_client, (void *)&client_sock);
            pthread_detach(thread); // 스레드 분리 (스레드 종료 시 자동 정리)
        } else {
            printf("최대 클라이언트 수를 초과했습니다.\n");
            close(client_sock); // 초과된 클라이언트 연결 닫기
        }
        pthread_mutex_unlock(&clients_mutex);   // 뮤텍스 잠금 해제
    }
    close(server_sock);
    return 0;
}

/* 클라이언트를 처리하는 스레드 함수 */
void *handle_client(void *arg) {
    int sock = *((int *)arg);           // 클라이언트 소켓
    char nickname[NICKNAME_LEN] = {0};  // 닉네임 저장 배열
    char buffer[BUFFER_SIZE];
    int read_len;

    // 클라이언트로부터 닉네임 수신
    if ((read_len = recv(sock, nickname, NICKNAME_LEN, 0)) > 0) {
        nickname[read_len] = '\0';
        printf("%s가 연결되었습니다.\n", nickname);
    }

    // 클라이언트로부터 메시지 수신
    while ((read_len = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_len] = '\0';
    
        // 다른 클라이언트에 메시지 전송
        char full_message[BUFFER_SIZE];
        snprintf(full_message, sizeof(full_message), "%s: %s", nickname, buffer);

        for(int i = 0; i< MAX_CLIENTS;i++){
            if(clients[i]!= -1)
                send(clients[i],full_message, BUFFER_SIZE, 0);
        }
    }
    
    // 클라이언트와의 연결 종료 처리
    close(sock);
    printf("%s가 연결을 종료했습니다.\n", nickname);

    return NULL;
}