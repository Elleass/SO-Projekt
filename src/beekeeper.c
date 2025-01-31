/*******************************************************
 * beekeeper.c
 *
 * Implementacja wątku pszczelarza, który:
 *  - Obsługuje sygnały SIGUSR1 i SIGUSR2
 *  - Zwiększa/zmniejsza pojemność ula (wywołuje adjust_hive_capacity)
 *******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "beekeeper.h"
#include "hive.h"
#include "egg.h"   
#include "error_handling.h"

// Zewnętrzna flaga zatrzymania
extern volatile sig_atomic_t stop;
extern int N; // liczba pszczół, zadeklarowana w main.c

// Lokalne prototypy
static void handle_sigusr1(int signo);
static void handle_sigusr2(int signo);

/**
 * Funkcja wątku pszczelarza (beekepeer).
 *  - Rejestruje obsługę sygnałów SIGUSR1 i SIGUSR2
 *  - Oczekuje na sygnały (pause())
 */
void* beekeeper(void* arg)
{
    // Pozwalamy na anulowanie wątku
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // SIGUSR1 -> powiększ ul
    struct sigaction sa1;
    sa1.sa_handler = handle_sigusr1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa1, NULL) < 0)
    {
        perror("sigaction(SIGUSR1) error");
        pthread_exit(NULL);
    }

    // SIGUSR2 -> zmniejsz ul
    struct sigaction sa2;
    sa2.sa_handler = handle_sigusr2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa2, NULL) < 0)
    {
        perror("sigaction(SIGUSR2) error");
        pthread_exit(NULL);
    }

    printf("[Beekeeper] Oczekiwanie na sygnały:\n");
    printf("  - SIGUSR1 (powiększ ul)\n");
    printf("  - SIGUSR2 (zmniejsz ul)\n");

    while (!stop)
    {
        pause();
    }

    printf("[Beekeeper] Kończę działanie wątku.\n");
    return NULL;
}

/**
 * Obsługa sygnału SIGUSR1
 *  - Powiększa ul do min(2*N, capacity * 2)
 */
static void handle_sigusr1(int signo)
{
    (void)signo;
    if (stop) return;

    lock_queue();
    int current = eggQueue->capacity;
    lock_queue(); // w oryginalnym kodzie było 2x lock_queue/unlock?
    unlock_queue();

    unlock_queue();

    int new_cap = current * 2;
    if (new_cap > 2 * N)
    {
        new_cap = 2 * N;
    }

    adjust_hive_capacity(new_cap);
}

/**
 * Obsługa sygnału SIGUSR2
 *  - Zmniejsza ul do max(1, capacity / 2)
 */
static void handle_sigusr2(int signo)
{
    (void)signo;
    if (stop) return;

    lock_queue();
    int current = eggQueue->capacity;
    unlock_queue();

    int new_cap = current / 2;
    if (new_cap < 1)
    {
        new_cap = 1;
    }

    adjust_hive_capacity(new_cap);
}
