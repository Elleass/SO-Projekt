#ifndef BEEKEPER_H
#define BEEKEPER_H
#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include "hive.h"
extern volatile sig_atomic_t stop;
extern int  capacity;


void* beekeeper(void* arg);

#endif