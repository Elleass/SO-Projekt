#ifndef HIVE_H
#define HIVE_H

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include "beekeper.h"
#include "egg.h"
#include "queen.h"
#include "error_handling.h"

// Globalne semafory wejść i ograniczenia ulu
extern sem_t *ul_wejscie;
extern sem_t wejscie1_kierunek;
extern sem_t wejscie2_kierunek;

// Globalna kolejka jaj (zawiera occupant_count i capacity)
extern EggQueue *eggQueue;

// Funkcje interakcji z "ulem"
void hive_leave(int id);
void hive_entry(int id);
void hive_state(void);
void adjust_hive_capacity(int new_capacity);

#endif // HIVE_H
