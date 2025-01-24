#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "../include/bee.h" 
#include "../include/hive.h"

void cleanup() {
    sem_destroy(&ul_wejscie);
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);
    printf("Semafory zostały zniszczone. Zakończenie programu.\n");
}

sem_t ul_wejscie;
sem_t wejscie1_kierunek;
sem_t wejscie2_kierunek;
volatile sig_atomic_t stop = 0; // Flag for graceful termination

void handle_sigint(int sig) {
    stop = 1;
    printf("\nSIGINT received. Exiting...\n");
}

int main() {
    int capacity = 0;   //pojemnosc ulu
    int start_bees = 0; //ilosc pszczol na poczatku programu, max(capacity /2)

    signal(SIGINT, handle_sigint); // Register signal handler

    printf("Podaj maksymalną liczbę pszczół w ulu: ");
    if (scanf("%d", &capacity) != 1 || capacity <= 0) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej większej od zera.\n");
        exit(EXIT_FAILURE);
    }
    printf("Maksymalna liczba pszczół w ulu: %d\n", capacity);

    printf("Podaj startową liczbę pszczół (maksymalnie %d): ", capacity / 2);
    if (scanf("%d", &start_bees) != 1 || start_bees <= 0 || start_bees > capacity / 2) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Startowa liczba pszczół musi być liczbą całkowitą z zakresu 1-%d.\n", capacity / 2);
        exit(EXIT_FAILURE);
    }

    pthread_t threads[1000]; // Allocate large enough array for threads
    int bee_count = start_bees;

    sem_init(&ul_wejscie, 0, capacity); //liczy liczbe pszczol w ulu
    sem_init(&wejscie1_kierunek, 0, 1); //wejscie nr 1
    sem_init(&wejscie2_kierunek, 0, 1); //wejscie nr 2

    // Tworzenie początkowych wątków pszczół
    for (int i = 0; i < start_bees; i++) {
        Bee* b = createBee(i + 1, 2, 3); // 2 sekundy w ulu, 3 wizyty
        if (pthread_create(&threads[i], NULL, bee_life, b) != 0) {
            fprintf(stderr, "Błąd: Nie udało się utworzyć wątku dla pszczoły %d.\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    pid_t p = fork();
    if (p < 0) {
        perror("Nie udało się utworzyć fork'a");
        exit(EXIT_FAILURE);
    } else if (p == 0) {
        // Queen process
        printf("To jest queen proces, PID: %d \n", getpid());
        while (!stop) {
        }
    } else {
            
        }

    // Oczekiwanie na zakończenie wszystkich wątków
    for (int i = 0; i < bee_count; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Błąd: Nie udało się zakończyć wątku dla pszczoły %d.\n", i + 1);
        }
    }

    cleanup(); // Zniszcz semafory
    return 0;
}
