#ifndef BEE_H
#define BEE_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Dynamiczna tablica wątków pszczół (globalnie)
extern pthread_t *bee_threads;
extern int bee_thread_count;
extern int bee_thread_capacity;

// Unikalny identyfikator nowej pszczoły
extern int next_bee_id;

// Mutex chroniący tablicę wątków
extern pthread_mutex_t bee_list_mutex;

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;

// Funkcje tworzenia i zarządzania pszczołami
Bee* createBee(int id, int time_in_hive, int visits_left);
void create_a_bee(Bee* b);
void* bee_life(void* arg);
int register_bee_thread(pthread_t thread);

#endif // BEE_H
