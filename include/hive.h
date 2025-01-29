#ifndef HIVE_H
#define HIVE_H
#include "beekeper.h"
#include "egg.h"
#include "queen.h"
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h> 

extern sem_t *ul_wejscie;
extern sem_t wejscie1_kierunek;
extern sem_t wejscie2_kierunek;
extern EggQueue *eggQueue;

void hive_leave(int id);
void hive_entry(int id);
void hive_state(void);
void adjust_hive_capacity(int new_capacity);

#endif