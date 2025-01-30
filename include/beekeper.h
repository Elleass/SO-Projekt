#ifndef BEEKEPER_H
#define BEEKEPER_H

#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "hive.h"
#include "bee.h"

// Wątek pszczelarza (obsługuje sygnały SIGUSR1 i SIGUSR2)
void* beekeeper(void* arg);

#endif // BEEKEPER_H
