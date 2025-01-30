#include "egg.h"
#include "error_handling.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Id-ki segmentu pamięci i semafora SysV
int shm_id = -1;
int sem_id = -1;

// Wskaźnik globalny zdefiniowany w main.c
// extern EggQueue *eggQueue;  // Już mamy w innym pliku

/**
 * Inicjalizacja segmentu pamięci wspólnej dla EggQueue (including occupant_count, capacity) 
 * + semafor SysV do synchronizacji.
 */
Error initSharedEggQueue(EggQueue **queue) {
    // Tworzymy segment pamięci wspólnej
    shm_id = shmget(IPC_PRIVATE, sizeof(EggQueue), IPC_CREAT | 0666);
    if (shm_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create shared memory"};
    }

    *queue = (EggQueue *)shmat(shm_id, NULL, 0);
    if (*queue == (void *)-1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to attach shared memory"};
    }

    // Inicjalizacja pól w strukturze
    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0;
    (*queue)->occupant_count = 0;
    (*queue)->capacity = 0;     // domyślna 0, ustawiana potem w main

    // Tworzymy semafor SysV (1 sztuka) – do blokowania kolejki
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create SysV semaphore"};
    }
    // Ustawiamy jego wartość na 1 (binarna blokada)
    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        return (Error){ERR_MEMORY_ALLOC, "Failed to set SysV semaphore value"};
    }

    return (Error){ERR_SUCCESS, "EggQueue initialized successfully"};
}

/**
 * Usuwamy segment pamięci i semafor
 */
void destroySharedEggQueue() {
    // Odłączamy i usuwamy segment
    if (shm_id != -1) {
        shmctl(shm_id, IPC_RMID, NULL); 
    }
    // Usuwamy semafor SysV
    if (sem_id != -1) {
        semctl(sem_id, 0, IPC_RMID);
    }
}

/**
 * Wstawianie jajka do kolejki + occupant_count++
 */
int enqueueEgg(EggQueue* q, Egg egg) {
    lock_queue();

    if (q->size == MAX_EGGS) {
        unlock_queue();
        return -1; // kolejka pełna
    }

    // occupant_count++ (jajko też zajmuje miejsce)
    q->occupant_count++;

    q->rear = (q->rear + 1) % MAX_EGGS;
    q->eggs[q->rear] = egg;
    q->size++;

    unlock_queue();
    return 0; 
}

/**
 * Wyjmowanie jajka z kolejki + occupant_count--
 */
int dequeueEgg(EggQueue* q, Egg* egg) {
    lock_queue();

    if (q->size == 0) {
        unlock_queue();
        return -1; // brak jaj
    }

    if (q->occupant_count > 0) {
        q->occupant_count--;
    }

    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;

    unlock_queue();
    return 0;
}

/**
 * occupant_increment / occupant_decrement
 * (dla pszczół – jeśli nie korzystają z enqueueEgg/dequeueEgg).
 */
void occupant_increment(void) {
    lock_queue();
    eggQueue->occupant_count++;
    unlock_queue();
}
void occupant_decrement(void) {
    lock_queue();
    if (eggQueue->occupant_count > 0) {
        eggQueue->occupant_count--;
    }
    unlock_queue();
}

/**
 * Blokada kolejki – semafor SysV
 */
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
