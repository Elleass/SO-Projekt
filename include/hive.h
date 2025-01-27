#ifndef HIVE_H
#define HIVE_H

#include <semaphore.h>

extern sem_t *ul_wejscie;
extern sem_t wejscie1_kierunek;
extern sem_t wejscie2_kierunek;

void hive_leave(int id);
void hive_entry(int id);
void hive_state(sem_t *ul_wejscie, int max_capacity);

#endif