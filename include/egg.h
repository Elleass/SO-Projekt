#ifndef EGG_H
#define EGG_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <unistd.h>

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
} EggQueue;

// Shared memory and semaphore management
EggQueue* initSharedEggQueue();
void destroySharedEggQueue();

// Queue operations
int enqueueEgg(EggQueue* queue, Egg egg);
int dequeueEgg(EggQueue* queue, Egg* egg);

#endif
