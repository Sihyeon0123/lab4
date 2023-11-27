#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENTS 5

pthread_mutex_t mutex;        // 뮤텍스 선언
pthread_cond_t condVariable;  // 조건 변수 선언

int clientCount = 0;           // 클라이언트 수
int broadcastMessage = 0;      // 방송할 메시지

void *clientThreadFunction(void *arg) {
    int clientID = *(int *)arg;

    while (1) {
        // 뮤텍스 락
        pthread_mutex_lock(&mutex);

        // 조건 변수 대기
        while (broadcastMessage == 0) {
            pthread_cond_wait(&condVariable, &mutex);
        }

        // 메시지 수신
        printf("Client %d received message: %d\n", clientID, broadcastMessage);

        // 뮤텍스 언락
        pthread_mutex_unlock(&mutex);

        // 잠시 대기
        usleep(500000);  // 0.5초 대기

        // 뮤텍스 락
        pthread_mutex_lock(&mutex);

        // 조건 변수 대기
        while (broadcastMessage != 0) {
            pthread_cond_wait(&condVariable, &mutex);
        }

        // 뮤텍스 언락
        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    pthread_t clientThreads[MAX_CLIENTS];
    int clientIDs[MAX_CLIENTS];

    // 뮤텍스 초기화
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        return 1;
    }

    // 조건 변수 초기화
    if (pthread_cond_init(&condVariable, NULL) != 0) {
        perror("Condition variable initialization failed");
        return 1;
    }

    // 클라이언트 스레드들 생성
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clientIDs[i] = i + 1;
        if (pthread_create(&clientThreads[i], NULL, clientThreadFunction, (void *)&clientIDs[i]) != 0) {
            perror("Error creating client thread");
            return 1;
        }
    }

    // 부모 스레드로부터 메시지 전송 요청을 받으면 모든 클라이언트에게 메시지를 방송
    while (1) {
        printf("Enter message to broadcast (0 to exit): ");
        scanf("%d", &broadcastMessage);

        if (broadcastMessage == 0) {
            break; // 0을 입력하면 종료
        }

        // 뮤텍스 락
        pthread_mutex_lock(&mutex);

        // 조건 변수 시그널 발생
        pthread_cond_broadcast(&condVariable);

        // 뮤텍스 언락
        pthread_mutex_unlock(&mutex);
    }

    // 클라이언트 스레드들의 종료를 기다림
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        pthread_join(clientThreads[i], NULL);
    }

    // 뮤텍스 소멸
    pthread_mutex_destroy(&mutex);

    // 조건 변수 소멸
    pthread_cond_destroy(&condVariable);

    return 0;
}
