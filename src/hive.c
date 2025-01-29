#include "hive.h"
#include "error_handling.h"


void hive_entry(int id) {
    int success = 0;

    while (!success && !stop) {
        // Try wejscie1_kierunek
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            if (sem_trywait(ul_wejscie) == 0) { // Reserve space in the hive
                occupant_increment(); // Update occupant count
                printf("\033[0;34mPszczoła %d wchodzi do ula wejściem 1.\033[0m\n", id);
                sem_post(&wejscie1_kierunek); // Release direction semaphore
                success = 1;
            } else {
                sem_post(&wejscie1_kierunek); // Release if ul_wejscie fails
                printf("\033[0;34mPszczoła %d czeka - ul pełny\033[0m\n", id);
                sleep(1);
            }
        }
        // Try wejscie2_kierunek
        else if (sem_trywait(&wejscie2_kierunek) == 0) {
            if (sem_trywait(ul_wejscie) == 0) { // Reserve space in the hive
                occupant_increment(); // Update occupant count
                printf("\033[0;34mPszczoła %d wchodzi do ula wejściem 2.\033[0m\n", id);
                sem_post(&wejscie2_kierunek); // Release direction semaphore
                success = 1;
            } else {
                sem_post(&wejscie2_kierunek); // Release if ul_wejscie fails
                printf("\033[0;34mPszczoła %d czeka - ul pełny\033[0m\n", id);
                sleep(1);
            }
        } else {
            printf("\033[0;33mPszczoła %d czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}


/**
 * Attempt to leave the hive:
 * - occupant_count--,
 * - sem_post(ul_wejscie) to free up a slot,
 * - direction semaphores handle concurrency out the door
 */
void hive_leave(int id) {
    int success = 0;

    while (!success && !stop) {
        // Try wejscie1_kierunek
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            occupant_decrement(); // Update occupant count
            sem_post(ul_wejscie); // Free hive space
            printf("\033[0;34mPszczoła %d wychodzi z ula wejściem 1.\033[0m\n", id);
            sem_post(&wejscie1_kierunek); // Release direction semaphore
            success = 1;
        }
        // Try wejscie2_kierunek
        else if (sem_trywait(&wejscie2_kierunek) == 0) {
            occupant_decrement(); // Update occupant count
            sem_post(ul_wejscie); // Free hive space
            printf("\033[0;34mPszczoła %d wychodzi z ula wejściem 2.\033[0m\n", id);
            sem_post(&wejscie2_kierunek); // Release direction semaphore
            success = 1;
        } else {
            printf("\033[0;33mPszczoła %d czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}


/**
 * Display the current state of the hive:
 * - occupant_count in eggQueue->occupant_count
 * - eggQueue->size (number of eggs)
 * - bees = occupant_count - eggQueue->size
 * - free_space = capacity - occupant_count
 */
void hive_state(void) {
    lock_queue();
    int occupant = eggQueue->occupant_count;
    int egg_count = eggQueue->size;
    unlock_queue();

    int bee_count = occupant - egg_count;
    if (bee_count < 0) bee_count = 0;

    int free_space = capacity - occupant;
    if (free_space < 0) free_space = 0; // should never go negative

    if (free_space == 0 && occupant >= capacity) {
        printf("\033[0;33mUl jest pełny.\033[0m | Bees: %d, Eggs: %d\n", bee_count, egg_count);
    } else {
        printf("\033[0;34mWolna przestrzeń w ulu: %d (zajęte: %d). Bees: %d, Eggs: %d\033[0m\n",
               free_space, occupant, bee_count, egg_count);
    }
}

/** Adjust the hive capacity (beekeeper signals) */
void adjust_hive_capacity(int new_capacity) {
    if (new_capacity < 1) {
        printf("Minimalna pojemność ula to 1. Ustawiam %d na 1.\n", new_capacity);
        new_capacity = 1;
    }
    // Just set the global capacity; occupant_count won't be touched here.
    if (new_capacity > capacity) {
        printf("\033[0;35mDodawanie ramek: zwiększanie pojemności ula do %d\033[0m\n", new_capacity);
    } else if (new_capacity < capacity) {
        printf("\033[0;35mUsuwanie ramek: zmniejszanie pojemności ula do %d\033[0m\n", new_capacity);
    } else {
        printf("Pojemność ula pozostaje taka sama: %d\n", new_capacity);
    }
    capacity = new_capacity;
}