#ifndef QUEEN_H
#define QUEEN_H

#include "egg.h"
#include "bee.h"
#include "hive.h"
#include "error_handling.h"
#include <signal.h>  // For sig_atomic_t
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> // Dodano nagłówek
#include <semaphore.h>
extern int next_bee_id;
extern volatile sig_atomic_t stop;
void queen_process(EggQueue* eggQueue);
void hatch_eggs(EggQueue* eggQueue);




#endif