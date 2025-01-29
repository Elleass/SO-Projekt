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



/** Initialize occupant_count=0, egg queue empty, plus a SysV semaphore for concurrency. */
Error initSharedEggQueue(EggQueue **queue) {
    // Create SysV shared memory segment
    shm_id = shmget(IPC_PRIVATE, sizeof(EggQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create shared memory"};
    }

    *queue = (EggQueue *)shmat(shm_id, NULL, 0);
    if (*queue == (void *)-1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to attach shared memory"};
    }

    // Initialize fields
    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0;
    (*queue)->occupant_count = 0;

    // Create a SysV semaphore for occupant_count + queue concurrency
    // Store in the global 'sem_id'
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create SysV semaphore"};
    }
    // Initialize the semaphore to 1 (binary lock)
    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to set SysV semaphore value"};
    }

    return (Error){ERR_SUCCESS, "EggQueue initialized successfully"};
}

void destroySharedEggQueue() {
    // Detach and remove the shared memory
    if (shm_id != -1) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("Failed to remove shared memory");
        }
    }
    // Remove the semaphore for occupant_count + queue
    if (sem_id != -1) {
        if (semctl(sem_id, 0, IPC_RMID) == -1) {
            perror("Failed to remove occupant_count SysV semaphore");
        }
    }
}

/**
 * Enqueue an Egg => occupant_count++ as well, because
 * an Egg also occupies the hive.
 * We lock the queue, check if queue is full, do occupant_count++,
 * then push the egg.
 */
int enqueueEgg(EggQueue* q, Egg egg) {
    lock_queue();

    if (q->size == MAX_EGGS) {
        unlock_queue();
        return -1; // queue is full
    }

    // occupant_count++ (the egg itself is occupying the hive)
    q->occupant_count++;

    q->rear = (q->rear + 1) % MAX_EGGS;
    q->eggs[q->rear] = egg;
    q->size++;

    unlock_queue();
    return 0; // success
}

/**
 * Dequeue an Egg => occupant_count--, because that Egg no longer occupies the hive.
 */
int dequeueEgg(EggQueue* q, Egg* egg) {
    lock_queue();

    if (q->size == 0) {
        unlock_queue();
        return -1; // no eggs
    }

    // occupant_count--
    if (q->occupant_count > 0) {
        q->occupant_count--;
    }

    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;

    unlock_queue();
    return 0; // success
}


void occupant_increment(void) {
    // Increment occupant_count in the shared memory
    eggQueue->occupant_count++;
}

void occupant_decrement(void) {
    if (eggQueue->occupant_count > 0) {
        eggQueue->occupant_count--;
    }
}

void lock_queue(void) {
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1) {
        perror("lock_queue: semop failed");
    }
}

void unlock_queue(void) {
    struct sembuf unlock_op = {0, 1, 0};
    if (semop(sem_id, &unlock_op, 1) == -1) {
        perror("unlock_queue: semop failed");
    }
}
