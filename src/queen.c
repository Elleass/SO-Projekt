#include "queen.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

/**
 * Proces "queen" – składa jaja, o ile jest miejsce w ulu.
 * Jako że `eggQueue->capacity` i `eggQueue->occupant_count` są w pamięci wspólnej,
 * królowa "widzi" aktualne wartości.
 */
void queen_process(EggQueue* eggQueue) {
    printf("inside queen_process (child PID: %d)\n", getpid());

    while (1) {
        if (stop) {
            _exit(0);
        }

        // Odczyt bieżącej liczby zajętych miejsc
        lock_queue();
        int occupant = eggQueue->occupant_count;
        int cap      = eggQueue->capacity;
        unlock_queue();

        if (occupant >= cap) {
            printf("Krolowa: Ul jest pelny, nie mozna zlozyc jaj.\n");
            sleep(1);
            continue;
        }

        // Symulacja czasu między złożeniem kolejnych jaj
        for (int i = 0; i < 5; i++) {
            if (stop) _exit(0);
            sleep(1);
        }

        // Tworzymy nowe jajo
        Egg new_egg;
        new_egg.id = next_bee_id++;
        new_egg.hatch_time = 5;  // np. 5 sekund

        // Spróbuj dołączyć do kolejki
        if (enqueueEgg(eggQueue, new_egg) == 0) {
            printf("\033[0;36mKrolowa: Zlozono jajo ID: %d (wykluje sie za %d s)\033[0m\n",
                   new_egg.id, new_egg.hatch_time);
        } else {
            printf("\033[0;33mKrolowa: Kolejka jaj jest pelny nie mozna zlozyc jaja\033[0m\n");
        }
    }
}

/**
 * Proces "hatch" – wyciąga jaja z kolejki (dequeue) i tworzy nowe wątki pszczół.
 */
void hatch_eggs(EggQueue* eggQueue) {
    printf("utworzono hatch_eggs (child PID: %d)\n", getpid());

    while (1) {
        if (stop) {
            _exit(0);
        }

        sleep(1);

        Egg hatched_egg;
        if (dequeueEgg(eggQueue, &hatched_egg) == 0) {
            // Wyklute jajo -> pszczoła
            printf("\033[0;32mWykluwanie: Jajko ID: %d wyklulo sie w pszczole!\033[0m\n", hatched_egg.id);

            Bee* new_bee = createBee(hatched_egg.id, 10, 3);
            if (!new_bee) {
                fprintf(stderr, "Failed to create new Bee object\n");
                continue;
            }
            create_a_bee(new_bee);
        } else {
            // Brak jaj
        }
    }
}
