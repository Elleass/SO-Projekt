#include "queen.h"


// queen
void queen_process(EggQueue* eggQueue) {
    printf("inside queen_process (child PID: %d)\n", getpid());

    while (1) {
        if (stop) {
            _exit(0);
        }

        // Check occupant_count vs. capacity
        lock_queue();
        int occupant = eggQueue->occupant_count;
        unlock_queue();

        if (occupant >= capacity) {
            printf("Queen: Hive is full, cannot lay eggs now\n");
            sleep(1);
            continue;
        }

        // simulate time between laying eggs
        for (int i = 0; i < 5; i++) {
            if (stop) _exit(0);
            sleep(1);
        }

        Egg new_egg;
        new_egg.id = next_bee_id++;
        new_egg.hatch_time = 5;

        // Attempt to enqueue the egg
        if (enqueueEgg(eggQueue, new_egg) == 0) {
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

        // Attempt to dequeue an egg
        if (dequeueEgg(eggQueue, &hatched_egg) == 0) {
            printf("\033[0;32mHatch process: Egg ID %d hatched into a bee!\033[0m\n", hatched_egg.id);

            // Create new Bee
            Bee* new_bee = createBee(hatched_egg.id, 10, 3);
            if (!new_bee) {
                fprintf(stderr, "Failed to create new Bee object\n");
                continue;
            }

            // Use create_a_bee to handle thread creation and registration
            create_a_bee(new_bee);
        } else {
            // No eggs to hatch right now
        }
    }
}
