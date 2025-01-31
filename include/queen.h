/*******************************************************
 * queen.h
 *
 * Deklaracje procesów:
 *  - queen_process (składanie jaj)
 *  - hatch_eggs (wylęganie)
 *******************************************************/

#ifndef QUEEN_H
#define QUEEN_H

#include "egg.h"
#include "bee.h"
#include <signal.h>

extern volatile sig_atomic_t stop;
extern int next_bee_id;
// Funkcje procesów
void queen_process(EggQueue* eggQueue);
void hatch_eggs(EggQueue* eggQueue);

#endif // QUEEN_H
