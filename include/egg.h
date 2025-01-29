#ifndef EGG_H
#define EGG_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <unistd.h>
#include "error_handling.h"  // Include for Error type

#define MAX_EGGS 100




typedef struct Egg {
    int id;
    int hatch_time;
} Egg;

typedef struct EggQueue {
    Egg eggs[MAX_EGGS];
    int front;
    int rear;
    int size;
    int occupant_count; 
} EggQueue;

extern EggQueue *eggQueue;
// Shared memory and semaphore management
Error initSharedEggQueue(EggQueue** queue);
void destroySharedEggQueue();

// Queue operations
int enqueueEgg(EggQueue* queue, Egg egg);
int dequeueEgg(EggQueue* queue, Egg* egg);
void occupant_increment(void);
 void occupant_decrement(void);

 void lock_queue(void);
void unlock_queue(void);

#endif
