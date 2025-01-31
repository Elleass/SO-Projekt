/*******************************************************
 * bee.h
 *
 * Deklaracja struktury i funkcji związanych z pszczołami.
 * Pszczoły istnieją w wątkach (pthread).
 *******************************************************/

#ifndef BEE_H
#define BEE_H

#include <pthread.h>
#include <signal.h>  // dla sig_atomic_t

// Struktura opisująca pszczołę
typedef struct {
    int id;
    int time_in_hive;   // ile sekund pszczoła przebywa w ulu przy jednej wizycie
    int visits_left;    // ile jeszcze wizyt w ulu ma wykonać
} Bee;

// --- Zmienne globalne (definiowane w main.c) ---
extern pthread_t *bee_threads;          // tablica wątków pszczół
extern int bee_thread_count;            // liczba wątków w tablicy
extern int bee_thread_capacity;         // rozmiar zaalokowanej tablicy
extern pthread_mutex_t bee_list_mutex;  // mutex do ochrony tablicy wątków
extern int next_bee_id;                 // globalny ID dla nowych pszczół
extern volatile sig_atomic_t stop;      // flaga stopu (zatrzymanie programu)

// --- Funkcje pszczoły ---
Bee* createBee(int id, int time_in_hive, int visits_left);
void create_a_bee(Bee* b);
void* bee_life(void* arg);
int register_bee_thread(pthread_t thread);

#endif // BEE_H
