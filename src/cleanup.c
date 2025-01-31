/*******************************************************
 * cleanup.c
 *
 * Implementacja funkcji sprzątającej.
 * Zatrzymuje procesy queen, hatch, wątek pszczelarza
 * i wszystkie wątki pszczół, niszczy semafory itd.
 *******************************************************/

#include "cleanup.h"
#include "egg.h"
#include "bee.h"
#include "hive.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

// Zmienna globalna
extern pid_t queen_pid;
extern pid_t hatch_pid;
extern pthread_t beekeeper_thread;
extern pthread_t *bee_threads;
extern int bee_thread_count;
extern int bee_thread_capacity;
extern pthread_mutex_t bee_list_mutex;
extern volatile sig_atomic_t stop;

// Semafory
extern sem_t *ul_wejscie;
extern sem_t wejscie1_kierunek;
extern sem_t wejscie2_kierunek;

void cleanup(void)
{
    printf("Starting cleanup...\n");

    if (queen_pid > 0)
    {
        printf("Terminating queen process (PID: %d)...\n", queen_pid);
        kill(queen_pid, SIGTERM);
        waitpid(queen_pid, NULL, 0);
        printf("Queen process terminated.\n");
    }

    if (hatch_pid > 0)
    {
        printf("Terminating hatch process (PID: %d)...\n", hatch_pid);
        kill(hatch_pid, SIGTERM);
        waitpid(hatch_pid, NULL, 0);
        printf("Hatch process terminated.\n");
    }

    if (beekeeper_thread)
    {
        printf("Canceling beekeeper thread...\n");
        pthread_cancel(beekeeper_thread);
        pthread_join(beekeeper_thread, NULL);
        printf("Beekeeper thread terminated.\n");
    }

    //Zatrzymanie wszystkich wątków pszczół
    pthread_mutex_lock(&bee_list_mutex);
    printf("Canceling %d bee threads...\n", bee_thread_count);
    for (int i = 0; i < bee_thread_count; i++)
    {
        pthread_cancel(bee_threads[i]);
    }
    for (int i = 0; i < bee_thread_count; i++)
    {
        pthread_join(bee_threads[i], NULL);
    }
    free(bee_threads);
    bee_threads = NULL;
    bee_thread_capacity = 0;
    bee_thread_count = 0;
    pthread_mutex_unlock(&bee_list_mutex);
    printf("Bee threads cleaned up.\n");

    //Usunięcie semaforów
    printf("Destroying semaphores...\n");
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);

    if (ul_wejscie != MAP_FAILED)
    {
        sem_destroy(ul_wejscie);
        munmap(ul_wejscie, sizeof(sem_t));
    }
    printf("Semaphores destroyed.\n");

    //Zwolnienie pamięci współdzielonej dla EggQueue
    if (eggQueue)
    {
        printf("Destroying egg queue...\n");
        destroySharedEggQueue();  // zwalnia segment i semafor SysV
        eggQueue = NULL;
        printf("Egg queue destroyed.\n");
    }

    printf("Cleanup completed. Exiting.\n");
}
