/*******************************************************
 * egg.h
 *
 * Deklaracja struktur i funkcji dotyczących "jaj" (Egg).
 * W tym: kolejka jaj (EggQueue) w pamięci współdzielonej.
 *******************************************************/

#ifndef EGG_H
#define EGG_H

#include "error_handling.h"

#define MAX_EGGS 100

// Struktura opisująca pojedyncze jajo
typedef struct Egg {
    int id;
    int hatch_time; // czas do wyklucia (w sekundach)
} Egg;

/**
 * Struktura kolejki jaj w pamięci współdzielonej:
 *  - eggs[]: tablica jaj
 *  - front, rear, size: pomocnicze do kolejki cyklicznej
 *  - occupant_count: liczba zajętych miejsc (jaj + pszczół)
 *  - capacity: maksymalna pojemność ula
 */
typedef struct EggQueue {
    Egg eggs[MAX_EGGS];
    int front;
    int rear;
    int size;

    int occupant_count; // ilu aktualnie jest "rezydentów" (pszczoły+jaja)
    int capacity;       // pojemność ula
} EggQueue;

extern EggQueue *eggQueue;

Error initSharedEggQueue(EggQueue** queue);
void destroySharedEggQueue(void);

int enqueueEgg(EggQueue* queue, Egg egg);
int dequeueEgg(EggQueue* queue, Egg* egg);

void occupant_increment(void);
void occupant_decrement(void);

void lock_queue(void);
void unlock_queue(void);

#endif // EGG_H
