#include "hive.h"
#include "error_handling.h"


void hive_entry(int id) {
    int success = 0;

    while (!success && !stop) {
        int hive_space;
        if (sem_getvalue(ul_wejscie, &hive_space) != 0) {
            handle_error((Error){ERR_SEMAPHORE_INIT, "sem_getvalue failed in hive_entry"});
            return;
        }
        if (hive_space <= 0) {
            printf("\033[0;34mPszczoła %d czeka w ulu brak miejsca\033[0m\n", id);
            sleep(1);
            continue;
        }
        // Try wejscie1
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_wait(ul_wejscie); // Reserve space
            printf("\033[0;34mPszczoła %d wchodzi do ula wejściem 1.\033[0m\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        // Try wejscie2
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_wait(ul_wejscie);
            printf("\033[0;34mPszczoła %d wchodzi do ula wejściem 2.\033[0m\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        } else {
            printf("\033[0;33mPszczoła %d czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}

void hive_leave(int id) {
    int success = 0;

    while (!success && !stop) {
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d wychodzi z ula wejściem 1.\033[0m\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d wychodzi z ula wejściem 2.\033[0m\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        } else {
            printf("\033[0;33mPszczoła %d czeka w ulu brak miejsca\033[0m\n", id);
            sleep(1);
        }
    }
}

void hive_state(sem_t *ul_wejscie, int max_capacity) {
    int value;
    if (sem_getvalue(ul_wejscie, &value) != 0) {
        handle_error((Error){ERR_SEMAPHORE_INIT, "sem_getvalue failed in hive_state"});
        return;
    }

    // Occupancy = max_capacity - value
    int occupancy = max_capacity - value;
    // Egg count is eggQueue->size
    int egg_count = 0;
    if (eggQueue) {
        egg_count = eggQueue->size;
    }
    // Bee count is occupancy - egg_count (since 1 egg = 1 occupant)
    int bee_count = occupancy - egg_count;
    if (bee_count < 0) {
        // Shouldn't happen if everything is consistent, but just in case
        bee_count = 0;
    }

    if (value == 0) {
        printf("\033[0;33mUl jest pełny.\033[0m | Bees: %d, Eggs: %d\n", bee_count, egg_count);
    } else {
        printf("\033[0;34mWolna przestrzeń w ulu: %d (zajęte: %d). Bees: %d, Eggs: %d\033[0m\n",
               value, occupancy, bee_count, egg_count);
    }
}
void adjust_hive_capacity(int new_capacity) {
    int current_value;
    if (sem_getvalue(ul_wejscie, &current_value) != 0) {
        handle_error((Error){ERR_SEMAPHORE_INIT, "sem_getvalue failed in adjust_hive_capacity"});
        return;
    }

    if (new_capacity > current_value) {
        printf("\033[0;35mDodawanie ramek: zwiększanie pojemności ula do %d\033[0m\n", new_capacity);
        for (int i = 0; i < (new_capacity - current_value); i++) {
            sem_post(ul_wejscie);
        }
    } else if (new_capacity < current_value) {
        // Ensure new capacity is not less than 1
        if (new_capacity < 1) {
            printf("Minimalna pojemność ula to 1. Ustawianie %d na 1.\n", new_capacity);
            new_capacity = 1;
        }
        printf("\033[0;35mUsuwanie ramek: zmniejszanie pojemności ula do %d\033[0m\n", new_capacity);
        for (int i = 0; i < (current_value - new_capacity); i++) {
            sem_wait(ul_wejscie);
        }
    } else {
        printf("Pojemność ula pozostaje taka sama: %d\n", new_capacity);
    }
}