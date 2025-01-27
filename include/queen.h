#ifndef QUEEN_H
#define QUEEN_H

#include "egg.h"
#include "bee.h"
#include "hive.h"
#include <signal.h>  // For sig_atomic_t
#include <sys/wait.h>
extern int next_bee_id;
extern volatile sig_atomic_t stop;
void* hatch_eggs(void* arg);
void* queen_process(void* arg);


#endif