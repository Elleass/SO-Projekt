/*******************************************************
 * beekeeper.h
 *
 * Deklaracje wątku pszczelarza, który reaguje na sygnały
 * SIGUSR1 (powiększ ul) i SIGUSR2 (pomniejsz ul).
 *******************************************************/

#ifndef BEEKEEPER_H
#define BEEKEEPER_H

void* beekeeper(void* arg);

#endif // BEEKEEPER_H
