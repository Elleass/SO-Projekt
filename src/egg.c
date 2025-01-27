#include "egg.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int shm_id = -1;
int sem_id = -1;

EggQueue* initSharedEggQueue() {
    // Create shared memory segment
    shm_id = shmget(IPC_PRIVATE, sizeof(EggQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory
    EggQueue* queue = (EggQueue*)shmat(shm_id, NULL, 0);
    if (queue == (void*)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }

    // Initialize queue in shared memory
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;

    // Initialize semaphores
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Failed to create semaphore");
        exit(EXIT_FAILURE);
    }
    semctl(sem_id, 0, SETVAL, 1); // Set initial semaphore value

    return queue;
}

void destroySharedEggQueue() {
    // Detach shared memory
    if (shm_id != -1) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Failed to remove shared memory");
        }
    }

    // Remove semaphore
    if (sem_id != -1) {
        if (semctl(sem_id, 0, IPC_RMID) == -1) {
            perror("Failed to remove semaphore");
        }
    }
}

int enqueueEgg(EggQueue* q, Egg egg) {
    // Lock semaphore
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1) {
        perror("Failed to lock semaphore");
        return -1;
    }

    if (q->size == MAX_EGGS) {
        // Queue is full
        struct sembuf unlock_op = {0, 1, 0};
        semop(sem_id, &unlock_op, 1);
        return -1;
    }

    q->rear = (q->rear + 1) % MAX_EGGS;
    q->eggs[q->rear] = egg;
    q->size++;

    // Unlock semaphore
    struct sembuf unlock_op = {0, 1, 0};
    semop(sem_id, &unlock_op, 1);

    return 0; // Success
}

int dequeueEgg(EggQueue* q, Egg* egg) {
    // Lock semaphore
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1) {
        perror("Failed to lock semaphore");
        return -1;
    }

    if (q->size == 0) {
        // Queue is empty
        struct sembuf unlock_op = {0, 1, 0};
        semop(sem_id, &unlock_op, 1);
        return -1;
    }

    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;

    // Unlock semaphore
    struct sembuf unlock_op = {0, 1, 0};
    semop(sem_id, &unlock_op, 1);

    return 0; // Success
}
