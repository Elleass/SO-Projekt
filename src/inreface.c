#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>  // Dodanie deklaracji semaforów
#include "interface.h"  // Nagłówek z deklaracjami funkcji interfejsu
#include "hive.h"       // Dostęp do ul_wejscie
#include "egg.h"        // Dostęp do struktury EggQueu
// Bufor do logów
#define LOG_BUFFER_SIZE 1024
char log_buffer[LOG_BUFFER_SIZE];
int log_index = 0;

extern int capacity;       // Maksymalna pojemność ula
extern int num_starter_bees; // Liczba początkowych pszczół
extern sem_t *ul_wejscie;  // Semafor dla przestrzeni ula
extern EggQueue* eggQueue; // Kolejka jaj

void log_event(const char* event) {
    if (log_index + strlen(event) < LOG_BUFFER_SIZE) {
        snprintf(log_buffer + log_index, LOG_BUFFER_SIZE - log_index, "%s\n", event);
        log_index += strlen(event) + 1;
    }
}

void display_hive_state() {
    int bees_in_hive = 0, eggs_in_hive = 0;

    sem_getvalue(ul_wejscie, &bees_in_hive);
    bees_in_hive = capacity - bees_in_hive;

    if (eggQueue) {
        eggs_in_hive = eggQueue->size;
    }

    int bees_outside = num_starter_bees - bees_in_hive;

    printf("\n=== Stan ula ===\n");
    printf("Pszczoły w ulu: %d\n", bees_in_hive);
    printf("Liczba jaj w ulu: %d\n", eggs_in_hive);
    printf("Pszczoły poza ulem: %d\n", bees_outside);
    printf("=================\n");
}

void show_logs() {
    printf("\n=== Logi ===\n");
    printf("%s", log_buffer);
    printf("=================\n");
}

void show_main_menu() {
    int choice;

    while (1) {
        printf("\n=== Menu główne ===\n");
        printf("1. Wyświetl stan ula\n");
        printf("2. Wyświetl logi\n");
        printf("3. Wyjście\n");
        printf("===================\n");
        printf("Wybierz opcję: ");
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Nieprawidłowy wybór.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
        case 1:
            display_hive_state();
            break;
        case 2:
            show_logs();
            break;
        case 3:
            printf("Zamykanie programu.\n");
            exit(0);
        default:
            printf("Nieprawidłowy wybór, spróbuj ponownie.\n");
        }
    }
}
