#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_THREADS 5

pthread_mutex_t mutex;
pthread_cond_t cond;

char message[100];

void *client(void *threadid) {
    long tid;
    tid = (long)threadid;

    pthread_mutex_lock(&mutex);
    sprintf(message, "Hello from thread #%ld!", tid);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void *server(void *threadid) {
    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_mutex_lock(&mutex);
        while(strlen(message) == 0) {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("Broadcast: %s\n", message);
        memset(message, 0, sizeof(message));
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    pthread_t server_thread;
    int rc;
    long t;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    memset(message, 0, sizeof(message));

    rc = pthread_create(&server_thread, NULL, server, (void *)t);
    if (rc) {
        printf("ERROR: return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    for(t = 0; t < NUM_THREADS; t++) {
        rc = pthread_create(&threads[t], NULL, client, (void *)t);
        if (rc) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    pthread_join(server_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    pthread_exit(NULL);
}
