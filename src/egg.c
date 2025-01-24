#include "egg.h"


void initEggQueue(EggQueue* q) {
    q ->front = 0;
    q->rear = -1;
    q->size = 0;
    pthread_mutex_init(&q->lock , NULL); 
}


int enqueueEgg(EggQueue* q, Egg egg) {
    pthread_mutex_lock(&q->lock);
    if(q->size == MAX_EGGS) {
        pthread_mutex_unlock(&q->lock);
        return -1; //queue full, 
    }
    q->rear = (q->rear + 1) % q->size;
    q->eggs[q->rear] = egg;
    q->size++;
    pthread_mutex_unlock(&q->lock);
    return 0; //succeess
}

int dequeue(EggQueue* q, Egg* egg) {
    pthread_mutex_lock(&q->lock);
    if (q->size == 0) {
        pthread_mutex_unlock(&q->lock);
        return -1; // Queue empty
    }
    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;
    pthread_mutex_unlock(&q->lock);
    return 0; // Success
}