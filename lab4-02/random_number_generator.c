#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define GOAL 20

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int count = 0;
int shared_num = 0;
int is_num_available = 0;

void *generate_random_number(void *ptr) {
    srand(time(NULL));

    while (1) {
        pthread_mutex_lock(&lock);

        while (is_num_available)
            pthread_cond_wait(&cond, &lock);
        if(count == GOAL){
            break;
        }
        shared_num = rand() % 100;
        printf("생산자: %d 번째 값 %d 생성\n", count+1, shared_num);
        is_num_available = 1;

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }

    return 0;
}

void *print_number(void *ptr) {
    while (1) {
        pthread_mutex_lock(&lock);

        while (!is_num_available)
            pthread_cond_wait(&cond, &lock);

        printf("소비자: %d 번째 생성된 랜덤 값: %d\n", count+1, shared_num);
        is_num_available = 0;
        count++;
        if(count == GOAL){
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }

    return 0;
}

int main() {
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, generate_random_number, NULL);
    pthread_create(&thread2, NULL, print_number, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    return 0;
}
