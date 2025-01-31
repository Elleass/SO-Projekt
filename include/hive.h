/*******************************************************
 * hive.h
 *
 * Deklaracje funkcji związanych z "ulem":
 *  - Wejście/wyjście pszczół
 *  - Wyświetlanie stanu ula
 *  - Dostosowanie pojemności ula
 *******************************************************/

#ifndef HIVE_H
#define HIVE_H

#include <semaphore.h>
#include <signal.h>   // dla sig_atomic_t
#include "egg.h"

// Zewnętrzne semafory (zdefiniowane w main.c)
extern sem_t *ul_wejscie;          // ogranicza liczbę osobników w ulu
extern sem_t wejscie1_kierunek;    // semafor kierunku 1
extern sem_t wejscie2_kierunek;    // semafor kierunku 2

// Flaga zatrzymania (z main.c)
extern volatile sig_atomic_t stop;

// Funkcje dla pszczół
void hive_entry(int id);
void hive_leave(int id);
void hive_state(void);

// Zmiana pojemności ula
void adjust_hive_capacity(int new_capacity);

#endif // HIVE_H
