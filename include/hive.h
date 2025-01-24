#ifndef hive.h
#define hive.h

#include <semaphore.h>

extern sem_t ul_wejscie;
extern sem_t wejscie1_kierunek;
extern sem_t wejscie2_kierunek;

void hive_leave(int id);
void hive_entry(int id);

#endif