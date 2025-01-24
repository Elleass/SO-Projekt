#ifndef egg.h
#define egg.h

#include <pthread.h>


#define MAX_EGGS 100


typedef struct Egg{
    int id;
    int hatch_time;
}Egg;

typedef struct EggQueue {
    Egg eggs[MAX_EGGS];
    int front;
    int rear;
    int size;
    pthread_mutex_t lock;
}EggQueue;


int dequeue(EggQueue* q, Egg* egg);
int enqueueEgg(EggQueue* q, Egg egg);


#endif