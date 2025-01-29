
#include "bee.h"
#include "hive.h"
#include "error_handling.h"



Bee* createBee(int id, int time_in_hive, int visits_left) {
    Bee* new_bee = (Bee*)malloc(sizeof(Bee));
    if (!new_bee) {
        handle_error((Error){ERR_MEMORY_ALLOC, "Failed to allocate memory for Bee"});
        return NULL;
    }
    new_bee->id = id;
    new_bee->time_in_hive = time_in_hive;
    new_bee->visits_left = visits_left;
    return new_bee;
}

void* bee_life(void* arg) {
    Bee* bee = (Bee*)arg;
    if (!bee) {
        fprintf(stderr, "Invalid Bee pointer\n");
        return NULL;
    }

    printf("\033[0;32mPszczoła %d startuje w wątku.\033[0m\n", bee->id);

    while (bee->visits_left > 0 && !stop) {
        // Attempt to enter hive
        hive_entry(bee->id);
        hive_state(ul_wejscie, capacity);
        // Simulate time in hive
        sleep(bee->time_in_hive);
        // Attempt to leave hive
        hive_leave(bee->id);
        hive_state(ul_wejscie, capacity);
        bee->visits_left--;
        // Simulate outside hive time
        sleep(5);
    }

    printf("\033[0;32mPszczoła %d kończy życie.\033[0m\n", bee->id);

    // Unlock direction semaphores as a fallback
    sem_post(&wejscie1_kierunek);
    sem_post(&wejscie2_kierunek);

    free(bee);
    return NULL;
}
