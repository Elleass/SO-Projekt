/*******************************************************
 * queen.c
 *
 * Implementacja procesów:
 *  - queen_process: królowa (składa jaja, jeśli jest miejsce)
 *  - hatch_eggs: wylęga jaja z kolejki (tworzy nowe pszczoły)
 *******************************************************/

#include "queen.h"
#include "egg.h"
#include "bee.h"
#include "hive.h"
#include "error_handling.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>



/**
 * queen_process
 *  - Królowa składa jaja, o ile occupant_count < capacity.
 *  - Jaja trafiają do kolejki (enqueueEgg).
 */
void queen_process(EggQueue* eggQueue)
{
    printf("Uruchomiono queen_process (child PID: %d)\n", getpid());

    while (1)
    {
        if (stop) _exit(0);

        lock_queue();
        int occupant = eggQueue->occupant_count;
        int cap      = eggQueue->capacity;
        unlock_queue();

        if (occupant >= cap)
        {
            printf("Królowa: Ul jest pełny, nie można złożyć jaj.\n");
            sleep(1);
            continue;
        }

        // Symulacja przerwy między składaniem kolejnych jaj
        for (int i = 0; i < 5; i++)
        {
            if (stop) _exit(0);
            sleep(1);
        }

        // Tworzymy nowe jajo
        Egg new_egg;
        new_egg.id = next_bee_id++;
        new_egg.hatch_time = 5;  // np. 5 sekund do wyklucia

        // Spróbuj wstawić do kolejki
        if (enqueueEgg(eggQueue, new_egg) == 0)
        {
            printf("\033[0;36mKrólowa: Złożono jajo ID: %d (wykluje się za %d s)\033[0m\n",
                   new_egg.id, new_egg.hatch_time);
        }
        else
        {
            printf("\033[0;33mKrólowa: Kolejka jaj jest pełna, nie można złożyć jaja\033[0m\n");
        }
    }
}

/**
 * hatch_eggs
 *  - Proces wyciąga jaja z kolejki (dequeueEgg).
 *  - Każde wyjęte jajo "wykluwa się" -> tworzymy nową pszczołę w wątku.
 */
void hatch_eggs(EggQueue* eggQueue)
{
    printf("Uruchomiono hatch_eggs (child PID: %d)\n", getpid());

    while (1)
    {
        if (stop) _exit(0);

        // Czekamy do wyklucia jajka
        sleep(5);

        Egg hatched_egg;
        if (dequeueEgg(eggQueue, &hatched_egg) == 0)
        {
            // Wyklute jajo -> pszczoła
            printf("\033[0;32mWykluwanie: Jajko ID: %d wykluło się w pszczołę!\033[0m\n", hatched_egg.id);
            printf("[DEBUG] occupant_count=%d, egg_count=%d => bee_count=%d\n",
                eggQueue->occupant_count, eggQueue->size,
                eggQueue->occupant_count - eggQueue->size);

            Bee* new_bee = createBee(hatched_egg.id, 4, 1);
            if (!new_bee)
            {
                fprintf(stderr, "Failed to create new Bee object\n");
                continue;
            }
            create_a_bee(new_bee);
        }
        else
        {
            // Brak jaj w kolejce
        }
    }
}
