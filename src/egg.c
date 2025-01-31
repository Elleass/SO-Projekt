/*******************************************************
 * egg.c
 *
 * Implementacja:
 *  - Pamięci współdzielonej dla EggQueue (SysV shmget/shmat)
 *  - Semafora SysV do blokady kolejki (semget, semop)
 *  - Funkcji enqueueEgg / dequeueEgg
 *  - Pomocniczych occupant_increment / occupant_decrement
 *******************************************************/

#include "egg.h"
#include "error_handling.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Identyfikatory segmentu pamięci i semafora SysV
static int shm_id = -1;
static int sem_id = -1;

/**
 * Inicjalizacja pamięci współdzielonej i semafora SysV.
 * Tworzy segment do przechowywania EggQueue.
 */
Error initSharedEggQueue(EggQueue **queue)
{
    //Tworzymy segment pamięci współdzielonej
    shm_id = shmget(IPC_PRIVATE, sizeof(EggQueue), IPC_CREAT | 0666);
    if (shm_id == -1)
    {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create shared memory"};
    }

    //Dołączamy segment
    *queue = (EggQueue *)shmat(shm_id, NULL, 0);
    if (*queue == (void *)-1)
    {
        return (Error){ERR_MEMORY_ALLOC, "Failed to attach shared memory"};
    }

    //Inicjalizacja pól w strukturze EggQueue
    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0;
    (*queue)->occupant_count = 0;
    (*queue)->capacity = 0; //ustawione w main przez uzytkownika

    //Tworzymy semafor SysV (1 sztuka) – do blokowania kolejki
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (sem_id == -1)
    {
        return (Error){ERR_MEMORY_ALLOC, "Failed to create SysV semaphore"};
    }

    //Ustawiamy wartość semafora na 1 (binary lock)
    if (semctl(sem_id, 0, SETVAL, 1) == -1)
    {
        return (Error){ERR_MEMORY_ALLOC, "Failed to set SysV semaphore value"};
    }

    return (Error){ERR_SUCCESS, "EggQueue initialized successfully"};
}


void destroySharedEggQueue(void)
{
    if (shm_id != -1)
    {
        shmctl(shm_id, IPC_RMID, NULL);
    }
    if (sem_id != -1)
    {
        semctl(sem_id, 0, IPC_RMID);
    }
}

int enqueueEgg(EggQueue* q, Egg egg)
{
    lock_queue();

    if (q->size == MAX_EGGS)
    {
        unlock_queue();
        return -1;
    }

    q->occupant_count++;

    // Wstawienie do cyklicznej kolejki
    q->rear = (q->rear + 1) % MAX_EGGS;
    q->eggs[q->rear] = egg;
    q->size++;

    unlock_queue();
    return 0;
}


int dequeueEgg(EggQueue* q, Egg* egg)
{
    lock_queue();

    if (q->size == 0)
    {
        // Kolejka pusta
        unlock_queue();
        return -1;
    }

    if (q->occupant_count > 0)
    {
        q->occupant_count--;
    }

    *egg = q->eggs[q->front];
    q->front = (q->front + 1) % MAX_EGGS;
    q->size--;

    unlock_queue();
    return 0;
}


void occupant_increment(void)
{
    lock_queue();
    eggQueue->occupant_count++;
    unlock_queue();
}


void occupant_decrement(void)
{
    lock_queue();
    if (eggQueue->occupant_count > 0)
    {
        eggQueue->occupant_count--;
    }
    unlock_queue();
}


void lock_queue(void)
{
    struct sembuf lock_op = {0, -1, 0};
    if (semop(sem_id, &lock_op, 1) == -1)
    {
        perror("lock_queue: semop failed");
    }
}

void unlock_queue(void)
{
    struct sembuf unlock_op = {0, 1, 0};
    if (semop(sem_id, &unlock_op, 1) == -1)
    {
        perror("unlock_queue: semop failed");
    }
}
