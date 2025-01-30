#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "beekeper.h"       // Twój plik nagłówkowy (deklaracje funkcji)
#include "hive.h"           // Deklaracja adjust_hive_capacity()

// Zakładamy, że w global_variables.h lub innym miejscu masz:
extern int capacity;
extern int N;
extern volatile sig_atomic_t stop;

// Prototypy lokalnych handlerów sygnałów:
static void handle_sigusr1(int signo);
static void handle_sigusr2(int signo);

/********************************************
 * Wątek pszczelarza
 * - Rejestrujemy tu obsługę sygnałów SIGUSR1 i SIGUSR2
 * - Czekamy w pętli (pause()), aż otrzymamy sygnał
 ********************************************/
void* beekeeper(void* arg)
{
    // Pozwalamy na anulowanie wątku (wyjdzie np. przy stop=1 w cleanup)
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // Ustawiamy obsługę SIGUSR1 -> powiększ ul
    struct sigaction sa1;
    sa1.sa_handler = handle_sigusr1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa1, NULL) < 0) {
        perror("sigaction(SIGUSR1) error");
        pthread_exit(NULL);
    }

    // Ustawiamy obsługę SIGUSR2 -> zmniejsz ul
    struct sigaction sa2;
    sa2.sa_handler = handle_sigusr2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = 0;
    if (sigaction(SIGUSR2, &sa2, NULL) < 0) {
        perror("sigaction(SIGUSR2) error");
        pthread_exit(NULL);
    }

    printf("[Beekeeper] Oczekiwanie na sygnały:\n");
    printf("  - SIGUSR1 (kill -USR1 %d) => powiększ ul do min(2*N, capacity*2)\n", getpid());
    printf("  - SIGUSR2 (kill -USR2 %d) => zmniejsz ul do capacity/2\n", getpid());

    // Wątek pszczelarza będzie czekał (pause) na sygnały.
    // Gdy stop == 1 (np. po Ctrl+C), wychodzimy z pętli.
    while (!stop) {
        pause();  // czekamy na dowolny sygnał
    }

    printf("[Beekeeper] Kończę działanie wątku.\n");
    return NULL;
}

/********************************************
 * Obsługa sygnału SIGUSR1 (dokładanie ramek)
 ********************************************/
static void handle_sigusr1(int signo)
{
    // Jeśli program i tak się zwija, ignorujemy
    if (stop) return;

    // Powiększamy ul: new_capacity = 2 * capacity, maksymalnie do 2*N
    int new_cap = capacity * 2;
    if (new_cap > 2 * N) {
        new_cap = 2 * N;
    }
    adjust_hive_capacity(new_cap);
}

/********************************************
 * Obsługa sygnału SIGUSR2 (usuwanie ramek)
 ********************************************/
static void handle_sigusr2(int signo)
{
    if (stop) return;

    // Zmniejszamy ul: new_capacity = capacity / 2 (minimum 1)
    int new_cap = capacity / 2;
    if (new_cap < 1) {
        new_cap = 1;
    }
    adjust_hive_capacity(new_cap);
}
