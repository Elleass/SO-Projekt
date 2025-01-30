#ifndef BEEKEPER_H
#define BEEKEPER_H
#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <stddef.h>
#include "hive.h"
#include "bee.h"
// extern volatile sig_atomic_t stop;
// extern int  capacity;


void* beekeeper(void* arg);

#endif