#ifndef BEE_H
#define BEE_H
extern int capacity;

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
// Declaration of the global dynamic array for bee threads
extern pthread_t *bee_threads;
extern int bee_thread_count;
extern int bee_thread_capacity;
extern int next_bee_id;

// A mutex that protects the bee_threads array
extern pthread_mutex_t bee_list_mutex;

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;

 Bee* createBee(int id, int time_in_hive, int visits_left);
void create_a_bee(Bee* b);
void* bee_life(void* arg);
 int register_bee_thread(pthread_t thread);


#endif