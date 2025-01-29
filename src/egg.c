#include "egg.h"
#include "error_handling.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int shm_id = -1;
int sem_id = -1;

// Error structure is from error_handling.

Error initSharedEggQueue(EggQueue **queue) {
    shm_id = shmget(IPC_PRIVATE, sizeof(EggQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create shared memory"};
    }

    *queue = (EggQueue*) shmat(shm_id, NULL, 0);
    if (*queue == (void*)-1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to attach shared memory"};
    }

    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0;

    // Create a System V semaphore for queue concurrency
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create System V semaphore"};
    }

    // Initialize the semaphore value to 1 (binary semaphore)
    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to set semaphore value"};
    }

    return (Error){ERR_SUCCESS, "EggQueue initialized successfully"};
}

void destroySharedEggQueue() {
    // Detach/remove shared memory
    if (shm_id != -1) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Failed to remove shared memory");
        }
    }

    // Remove the semaphore
    if (sem_id != -1) {
        if (semctl(sem_id, 0, IPC_RMID) == -1) {
            perror("Failed to remove System V semaphore");
        }
    }
}

int enqueueEgg(EggQueue* q, Egg egg) {
    // Acquire lock
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1) {
        perror("enqueueEgg: Failed to lock semaphore");
        return -1;
    }

    if (q->size == MAX_EGGS) {
        // Release lock
        struct sembuf unlock_op = {0, 1, 0};
        semop(sem_id, &unlock_op, 1);
        return -1;
    }

    q->rear = (q->rear + 1) % MAX_EGGS;
    q->eggs[q->rear] = egg;
    q->size++;

    // Release lock
    struct sembuf unlock_op = {0, 1, 0};
    semop(sem_id, &unlock_op, 1);

    return 0; // success
}

int dequeueEgg(EggQueue* q, Egg* egg) {
    // Acquire lock
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1) {
        perror("dequeueEgg: Failed to lock semaphore");
        return -1;
    }

    if (q->size == 0) {
        // Release
        struct sembuf unlock_op = {0, 1, 0};
        semop(sem_id, &unlock_op, 1);
        return -1;
    }

    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;

    // Release lock
    struct sembuf unlock_op = {0, 1, 0};
    semop(sem_id, &unlock_op, 1);

    return 0; // success
}