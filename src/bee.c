/*******************************************************
 * bee.c
 *
 * Implementacja funkcji związanych z pszczołami:
 *  - Tworzenie obiektu Bee (createBee)
 *  - Zakładanie wątku pszczoły (create_a_bee)
 *  - Główna funkcja wątku pszczoły (bee_life)
 *  - Rejestrowanie wątku pszczoły w tablicy globalnej
 *******************************************************/

#include "bee.h"
#include "hive.h"
#include "error_handling.h"
#include "cleanup.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * createBee
 *  - Alokuje pamięć dla nowej pszczoły i inicjuje jej pola.
 */
Bee* createBee(int id, int time_in_hive, int visits_left)
{
    Bee* new_bee = (Bee*)malloc(sizeof(Bee));
    if (!new_bee)
    {
        handle_error((Error){ERR_MEMORY_ALLOC, "Failed to allocate memory for Bee"});
        return NULL;
    }
    new_bee->id = id;
    new_bee->time_in_hive = time_in_hive;
    new_bee->visits_left = visits_left;
    return new_bee;
}

/**
 * create_a_bee
 *  - Tworzy wątek pszczoły (bee_life) i rejestruje go w tablicy globalnej wątków.
 *  - Obsługuje ewentualne błędy (np. brak zasobów systemowych EAGAIN).
 */
void create_a_bee(Bee* b)
{
    pthread_t t;
    int rc = pthread_create(&t, NULL, bee_life, b);
    static int consecutive_failures = 0; // śledzi kolejne niepowodzenia

    if (rc != 0)
    {
        fprintf(stderr, "Failed to create bee thread (errno=%d)\n", rc);
        free(b);

        if (rc == EAGAIN)  // EAGAIN = brak zasobów
        {
            consecutive_failures++;
            if (consecutive_failures >= 3)
            {
                fprintf(stderr, "Repeated thread creation failures. Stopping program.\n");
                stop = 1;
                cleanup();
                exit(EXIT_FAILURE);
            }
        }
        return;
    }

    // Jeśli się udało, zerujemy licznik niepowodzeń
    consecutive_failures = 0;

    // Rejestrujemy wątek w tablicy
    if (register_bee_thread(t) < 0)
    {
        fprintf(stderr, "Out of memory storing bee thread.\n");
        pthread_cancel(t);
        free(b);
        stop = 1;
        cleanup();
        exit(EXIT_FAILURE);
    }
}

/**
 * register_bee_thread
 *  - Dodaje wątek do globalnej tablicy wątków (z dynamicznym powiększaniem).
 */
int register_bee_thread(pthread_t thread)
{
    pthread_mutex_lock(&bee_list_mutex);

    if (bee_thread_count == bee_thread_capacity)
    {
        int new_cap = (bee_thread_capacity == 0) ? 16 : (bee_thread_capacity * 2);
        pthread_t *tmp = realloc(bee_threads, new_cap * sizeof(pthread_t));
        if (!tmp)
        {
            pthread_mutex_unlock(&bee_list_mutex);
            return -1;
        }
        bee_threads = tmp;
        bee_thread_capacity = new_cap;
    }

    bee_threads[bee_thread_count++] = thread;
    pthread_mutex_unlock(&bee_list_mutex);
    return 0;
}

/**
 * bee_life
 *  - Główna funkcja wątku pszczoły.
 *  - W pętli: wchodzi do ula -> przebywa w nim -> wychodzi -> czeka poza ulem -> zmniejsza visits_left.
 */
void* bee_life(void* arg)
{
    Bee* bee = (Bee*)arg;
    if (!bee)
    {
        fprintf(stderr, "Invalid Bee pointer\n");
        return NULL;
    }

    printf("\033[0;32mPszczoła %d: startuje w wątku.\033[0m\n", bee->id);

    while (bee->visits_left > 0 && !stop)
    {
        hive_entry(bee->id);
        hive_state();

        sleep(bee->time_in_hive);

        hive_leave(bee->id);
        hive_state();

        bee->visits_left--;

        // Poza ulem (symulacja)
        sleep(5);
    }

    printf("\033[0;32mPszczoła %d: kończy życie.\033[0m\n", bee->id);

    // Na wszelki wypadek odblokowanie semaforów kierunku
    sem_post(&wejscie1_kierunek);
    sem_post(&wejscie2_kierunek);

    free(bee);
    return NULL;
}
