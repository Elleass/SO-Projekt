#include "../include/hive.h"
#include <stdio.h>
#include <unistd.h> // Dodano dla funkcji sleep

void hive_entry(int id) {
    int success = 0;
    int hive_space;

    while (!success) {
        // Check if there's space in the hive
        if (sem_getvalue(ul_wejscie, &hive_space) == 0 && hive_space <= 0) {
            printf("Pszczoła %d czeka, w ulu brak miejsca.\n", id);
            sleep(1); // Wait before retrying
            continue;
        }

        // Try to enter through wejscie1_kierunek
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_wait(ul_wejscie); // Reserve space in the hive
            printf("Pszczoła %d wchodzi do ula wejściem 1.\n", id);
            sem_post(&wejscie1_kierunek); // Release entry direction semaphore
            success = 1;

        // Try to enter through wejscie2_kierunek
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_wait(ul_wejscie); // Reserve space in the hive
            printf("Pszczoła %d wchodzi do ula wejściem 2.\n", id);
            sem_post(&wejscie2_kierunek); // Release entry direction semaphore
            success = 1;

        // Both entry points are occupied
        } else {
            printf("Pszczoła %d czeka, oba wejścia są zajęte.\n", id);
            sleep(1); // Wait before retrying
        }
    }
}

void hive_leave(int id) {
    int success = 0;
    while (!success) {
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_post(ul_wejscie); // Zwiększenie liczby dostępnych miejsc w ulu
            printf("Pszczoła %d wychodzi z ula wejściem 1.\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_post(ul_wejscie); // Zwiększenie liczby dostępnych miejsc w ulu
            printf("Pszczoła %d wychodzi z ula wejściem 2.\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        } else {
            printf("Pszczoła %d czeka, oba wyjścia są zajęte.\n", id);
            sleep(1);
        }
    }
}


void hive_state(sem_t *ul_wejscie, int max_capacity) {
    int value;

    // Get the current semaphore value
    if (sem_getvalue(ul_wejscie, &value) == 0) {
        if (value < 0) {
            printf("Ul jest pełny, liczba czekających pszczół %d\n", -value);
        } else if (value > max_capacity) {
            printf("Error: Semaphore value exceeds max capacity! Current value: %d\n", value);
        } else {
            int taken_spaces = max_capacity - value;
            printf("Przestrzeń zajęta: %d\n", taken_spaces);
            printf("Wolna przestrzeń w ulu: %d\n", value);
        }
    } else {
        perror("sem_getvalue failed"); // Error in retrieving semaphore value
    }
}
