#include "queen.h"


Error initSharedEggQueue(EggQueue **queue);
int enqueueEgg(EggQueue* q, Egg egg);
int dequeueEgg(EggQueue* q, Egg* egg);

// queen
void queen_process(EggQueue* eggQueue) {
    printf("inside queen_process (child PID: %d)\n", getpid());

    while (1) {
        if (stop) {
            _exit(0);
        }

        int available_space;
        if (sem_getvalue(ul_wejscie, &available_space) != 0) {
            handle_error((Error){ERR_SEMAPHORE_INIT, "sem_getvalue failed in queen_process"});
            _exit(1);
        }

        if (available_space <= 0) {
            printf("Queen: Hive is full, cannot lay eggs now\n");
            sleep(1);
            continue;
        }

        // Simulate time between laying eggs
        sleep(5);

        Egg new_egg;
        new_egg.id = next_bee_id++;
        new_egg.hatch_time = 5;

        // Attempt to enqueue the egg
        if (enqueueEgg(eggQueue, new_egg) == 0) {
            sem_wait(ul_wejscie); // Occupy space
            printf("\033[0;36mQueen: Laid egg ID %d, hatching in %d seconds\033[0m\n",
                   new_egg.id, new_egg.hatch_time);
        } else {
            printf("\033[0;33mQueen: Egg queue is full, cannot lay more eggs\033[0m\n");
        }
    }
}

void hatch_eggs(EggQueue* eggQueue) {
    printf("utworzono hatch_eggs (child PID: %d)\n", getpid());

    while (1) {
        if (stop) {
            _exit(0);
        }
        sleep(1);

        Egg hatched_egg;
        if (dequeueEgg(eggQueue, &hatched_egg) == 0) {
            printf("\033[0;32mHatch process: Egg ID %d hatched into a bee!\033[0m\n", hatched_egg.id);

            // Release space in hive (egg no longer occupies space)
            sem_post(ul_wejscie);

            // Create a new Bee
            Bee* new_bee = createBee(hatched_egg.id, 10, 3);
            if (!new_bee) {
                fprintf(stderr, "Failed to create new Bee object\n");
                continue;
            }

            pthread_t new_thread;
            if (pthread_create(&new_thread, NULL, bee_life, new_bee) == 0) {
                printf("\033[0;32mHatch process: Bee ID %d thread created.\033[0m\n", hatched_egg.id);
                pthread_detach(new_thread);
            } else {
                handle_error((Error){ERR_THREAD_FAILURE, "Failed to create thread for new Bee"});
                free(new_bee);
            }
        } else {
            // No eggs to hatch
        }
    }
}
