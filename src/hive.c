#include "../include/hive.h"
#include <stdio.h>
#include <unistd.h> // Dodano dla funkcji sleep

void hive_entry(int id) {
    int success = 0;
    while (!success) {
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_wait(&ul_wejscie);
            printf("Pszczoła %d wchodzi do ula wejściem 1.\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_wait(&ul_wejscie);
            printf("Pszczoła %d wchodzi do ula wejściem 2.\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        } else {
            printf("Pszczoła %d czeka, oba wejścia są zajęte.\n", id);
            sleep(1);
        }
    }
}

void hive_leave(int id) {
    int success = 0;
    while (!success) {
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            sem_post(&ul_wejscie); // Zwiększenie liczby dostępnych miejsc w ulu
            printf("Pszczoła %d wychodzi z ula wejściem 1.\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        } else if (sem_trywait(&wejscie2_kierunek) == 0) {
            sem_post(&ul_wejscie); // Zwiększenie liczby dostępnych miejsc w ulu
            printf("Pszczoła %d wychodzi z ula wejściem 2.\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        } else {
            printf("Pszczoła %d czeka, oba wyjścia są zajęte.\n", id);
            sleep(1);
        }
    }
}
