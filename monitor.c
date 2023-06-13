#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define N_CONSUMERS 3

typedef struct {
    pthread_cond_t data_ready;
    pthread_mutex_t lock;
    int ready;
    int data;
} monitor_t;

monitor_t monitor = {
    .lock = PTHREAD_MUTEX_INITIALIZER, 
    .data_ready = PTHREAD_COND_INITIALIZER,
    .ready = 0, 
    .data = 0
};

void* provide(void* arg) {
    srand(time(NULL));
    for (;;) {
        int data = rand();

        pthread_mutex_lock(&monitor.lock);

        if (monitor.ready == 1) {
            pthread_mutex_unlock(&monitor.lock);
            continue;
        }

        printf("provider: providing data: %d\n", data);
        monitor.data = data;
        monitor.ready = 1;

        pthread_cond_signal(&monitor.data_ready);
        pthread_mutex_unlock(&monitor.lock);
    }
}

void* consume(void* arg) {
    size_t id = (size_t) arg;

    while (1) {
        pthread_mutex_lock(&monitor.lock);

        while (monitor.ready == 0) {
            pthread_cond_wait(&monitor.data_ready, &monitor.lock);
        }
        int data = monitor.data;
        monitor.ready = 0;

        pthread_mutex_unlock(&monitor.lock);

        printf("consumer %lu: got data %d. sleeping for %d\n", id, data, data % 3);
        sleep(data % 3);
    }
}

int main() {
    pthread_t provider_tid;
    pthread_t consumer_tids[N_CONSUMERS];

    int result = pthread_create(&provider_tid, NULL, provide, NULL);
    if (result != 0) {
        perror("pthread_create");
        return 1;
    }

    for (size_t i = 0; i < N_CONSUMERS; i++) {
        int result = pthread_create(&consumer_tids[i], NULL, consume, (void*)i);
        if (result != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    pthread_join(provider_tid, NULL);
    for (size_t i = 0; i < N_CONSUMERS; i++) {
        pthread_join(consumer_tids[i], NULL);
    }
    return 0;
}