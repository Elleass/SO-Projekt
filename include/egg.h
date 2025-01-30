#ifndef EGG_H
#define EGG_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <unistd.h>
#include "error_handling.h"  // dla typu Error

#define MAX_EGGS 100

// Struktura opisująca pojedyncze jajo
typedef struct Egg {
    int id;
    int hatch_time;
} Egg;

/**
 * Struktura kolejki jaj w pamięci współdzielonej:
 *  - eggs[]: tablica jaj
 *  - front, rear, size: wskaźniki/rozmiar kolejki cyklicznej
 *  - occupant_count: liczba zajętych miejsc (jaj + pszczół) w ulu
 *  - capacity: aktualna pojemność ula
 */
typedef struct EggQueue {
    Egg eggs[MAX_EGGS];
    int front;
    int rear;
    int size;
    int occupant_count;
    int capacity;  // <--- PRZENIESIONE TU ZAMIAST GLOBALNEJ ZMIENNEJ
} EggQueue;

// Wskaźnik do kolejki (w pamięci współdzielonej)
extern EggQueue *eggQueue;

// Zarządzanie pamięcią współdzieloną (tworzenie, usuwanie)
Error initSharedEggQueue(EggQueue** queue);
void destroySharedEggQueue();

// Operacje na kolejce
int enqueueEgg(EggQueue* queue, Egg egg);
int dequeueEgg(EggQueue* queue, Egg* egg);

// Zwiększanie/zmniejszanie occupant_count (np. przez pszczoły)
void occupant_increment(void);
void occupant_decrement(void);

// Blokada kolejki (semafor SysV)
void lock_queue(void);
void unlock_queue(void);

#endif // EGG_H
