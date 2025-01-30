#ifndef QUEEN_H
#define QUEEN_H

#include "egg.h"
#include "bee.h"
#include "hive.h"
#include "error_handling.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

// Flaga sygnałowa do zatrzymania
extern volatile sig_atomic_t stop;

// Proces królowej (składanie jaj)
void queen_process(EggQueue* eggQueue);

// Proces wylęgania jaj
void hatch_eggs(EggQueue* eggQueue);

#endif // QUEEN_H
