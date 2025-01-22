#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/bee.h" // Upewnij się, że plik zawiera definicję createBee i bee_life.

int main() {
    int capacity = 0;
    int start_bees = 0;

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

    pthread_t threads[start_bees];

    // Tworzenie wątków
    for (int i = 0; i < start_bees; i++) {
        Bee* b = createBee(i + 1, 2, 3); // 2 sekundy w ulu, 3 wizyty
        if (pthread_create(&threads[i], NULL, bee_life, b) != 0) {
            fprintf(stderr, "Błąd: Nie udało się utworzyć wątku dla pszczoły %d.\n", i + 1);
            exit(EXIT_FAILURE);
        }
    }

    // Oczekiwanie na zakończenie wątków
    for (int i = 0; i < start_bees; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Błąd: Nie udało się zakończyć wątku dla pszczoły %d.\n", i + 1);
        }
    }

    return 0;
}
