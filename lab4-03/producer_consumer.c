#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 5
#define NUM_CONSUMERS 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void* producer(void* arg);
void* consumer(void* arg);

int main() {
    pthread_t producer_threads[NUM_PRODUCERS]; // 생산자
    pthread_t consumer_threads[NUM_CONSUMERS]; // 소비자

    // 생산자 및 소비자 스레드 생성
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_create(&producer_threads[i], NULL, producer, (void*)(intptr_t)i);
        pthread_create(&consumer_threads[i], NULL, consumer, (void*)(intptr_t)i);
    }

    // 생산자 스레드 종료 대기
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        pthread_join(producer_threads[i], NULL);
    }

    // 소비자 스레드 종료 대기
    for (int i = 0; i < NUM_CONSUMERS; ++i) {   
        pthread_join(consumer_threads[i], NULL);
    }

    return 0;
}

// 생산자 루틴
void* producer(void* arg) {
    int producer_id = (int)(intptr_t)arg;
    
    for(int i=0; i<BUFFER_SIZE; i++){
        // 뮤텍스 획득시도
        pthread_mutex_lock(&mutex);

        while (count == BUFFER_SIZE) {
            // 버퍼가 가득 찼을 때 대기
            pthread_cond_wait(&not_full, &mutex);
        }

        // 버퍼 채우기
        buffer[count++] = producer_id;
        printf("생산자%d가 %d를 채웠으며 총 %d번 채웠습니다!\n", producer_id, producer_id, i+1);
        
        // 생산 후 소비자에게 신호 보내기
        pthread_cond_signal(&not_empty);

        // 뮤텍스 해제
        pthread_mutex_unlock(&mutex);
    }
}

// 소비자 루틴
void* consumer(void* arg) {
    int consumer_id = (int)(intptr_t)arg;

    for(int i=0; i<BUFFER_SIZE; i++){
        // 뮤텍스 획득
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            // 버퍼가 비었을 때 대기
            pthread_cond_wait(&not_empty, &mutex);
        }

        // 버퍼 채우기
        printf("소비자%d가 %d를 소비하였으며 총 %d번 소비하였습니다.\n",consumer_id, buffer[--count], i+1);
        // 소비 후 생산자에게 신호 보내기
        pthread_cond_signal(&not_full);
        // 뮤텍스 해제
        pthread_mutex_unlock(&mutex);
    }
}