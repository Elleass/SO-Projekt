
#include "bee.h"
#include "hive.h"
#include "error_handling.h"

pthread_t *bee_threads = NULL;
int bee_thread_count = 0;
int bee_thread_capacity = 0;

pthread_mutex_t bee_list_mutex = PTHREAD_MUTEX_INITIALIZER;

Bee* createBee(int id, int time_in_hive, int visits_left) {
    Bee* new_bee = (Bee*)malloc(sizeof(Bee));
    if (!new_bee) {
        handle_error((Error){ERR_MEMORY_ALLOC, "Failed to allocate memory for Bee"});
        return NULL;
    }
    new_bee->id = id;
    new_bee->time_in_hive = time_in_hive;
    new_bee->visits_left = visits_left;
    return new_bee;
}


void create_a_bee(Bee* b) {
    pthread_t t;
    int rc = pthread_create(&t, NULL, bee_life, b);
    if (rc != 0) {
        fprintf(stderr, "Failed to create bee thread (errno=%d)\n", rc);
        free(b);
        stop = 1; // gracefully signal to stop
        return;
    }
    if (register_bee_thread(t) < 0) {
        fprintf(stderr, "Out of memory storing bee thread.\n");
        pthread_cancel(t);
        free(b);
        stop = 1;
        return;
    }
}

 int register_bee_thread(pthread_t thread) {
    pthread_mutex_lock(&bee_list_mutex);

    if (bee_thread_count == bee_thread_capacity) {
        int new_cap = (bee_thread_capacity == 0) ? 16 : bee_thread_capacity * 2;
        pthread_t *tmp = realloc(bee_threads, new_cap * sizeof(pthread_t));
        if (!tmp) {
            pthread_mutex_unlock(&bee_list_mutex);
            return -1;
        }
        bee_threads = tmp;
        bee_thread_capacity = new_cap;
    }

    bee_threads[bee_thread_count++] = thread;
    pthread_mutex_unlock(&bee_list_mutex);
    return 0;
}

void* bee_life(void* arg) {
    Bee* bee = (Bee*)arg;
    if (!bee) {
        fprintf(stderr, "Invalid Bee pointer\n");
        return NULL;
    }

    printf("\033[0;32mPszczoła %d startuje w wątku.\033[0m\n", bee->id);

    while (bee->visits_left > 0 && !stop) {
        // Attempt to enter hive
        hive_entry(bee->id);
        hive_state();
        // Simulate time in hive
        sleep(bee->time_in_hive);
        // Attempt to leave hive
        hive_leave(bee->id);
        hive_state();
        bee->visits_left--;
        // Simulate outside hive time
        sleep(5);
    }

    printf("\033[0;32mPszczoła %d kończy życie.\033[0m\n", bee->id);

    // Unlock direction semaphores as a fallback
    sem_post(&wejscie1_kierunek);
    sem_post(&wejscie2_kierunek);

    free(bee);
    return NULL;
}
