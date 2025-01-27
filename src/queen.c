#include "queen.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h> // Dodano nagłówek
#include <semaphore.h>

// Pointer to shared memory queue externed from main
extern EggQueue* eggQueue;

void* queen_process(void* arg) {
    (void)arg; // Oznaczenie argumentu jako nieużywanego
    printf("inside queen_process\n");
    while (1) {
        // Check for termination signal
        if (stop) {
            exit(0);
        }

        // Check if there's space in the hive
        int available_space;
    sem_getvalue(ul_wejscie, &available_space);
        printf("Queen: Available hive space: %d (PID: %d)\n", available_space, getpid());

        if (available_space <= 0) {
            printf("Queen: Hive is full, cannot lay more eggs right now\n");
            sleep(1); // Wait and retry
            continue;
        }

        sleep(5);

        Egg new_egg = { next_bee_id++, 1 };
        printf("Queen: Attempting to lay egg ID %d\n", new_egg.id);

        // Add egg to the shared queue
        if (enqueueEgg(eggQueue, new_egg) == 0) {
            // Egg successfully added, decrement hive space
            sem_wait(ul_wejscie);
            printf("Queen: Laid egg ID %d, hatching in %d seconds\n", new_egg.id, new_egg.hatch_time);
        } else {
            printf("Queen: Egg queue is full, cannot lay more eggs\n");
        }
    }
}

void* hatch_eggs(void* arg) {
    (void)arg; // Oznaczenie argumentu jako nieużywanego
    printf("utworzono hatch_eggs\n");

    while (1) {
        // Check for termination signal
        if (stop) {
            exit(0);
        }

        sleep(1);

        Egg hatched_egg;

        // Attempt to hatch eggs
        if (dequeueEgg(eggQueue, &hatched_egg) == 0) {
            printf("Hatch process: Egg ID %d has hatched into a bee!\n", hatched_egg.id);
            sem_post(ul_wejscie);
            Bee* new_bee = createBee(hatched_egg.id, 10, 3);
            pthread_t new_thread;
            if (pthread_create(&new_thread, NULL, bee_life, new_bee) == 0) {
                printf("Hatch process: Bee ID %d thread created.\n", hatched_egg.id);
                
    
            } else {
                fprintf(stderr, "Hatch process: Failed to create thread for Bee ID %d.\n", hatched_egg.id);
                free(new_bee);
            }

            // Release space in the hive after hatching
        } else {
            printf("Hatch process: No eggs to hatch at the moment.\n");
        }
    }
}
