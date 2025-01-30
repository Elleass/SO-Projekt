#include "hive.h"
#include "error_handling.h"

// Zakładamy, że w pliku nagłówkowym hive.h:
//   - extern EggQueue *eggQueue;       // W pamięci współdzielonej
//   - extern sem_t *ul_wejscie;
//   - extern sem_t wejscie1_kierunek;
//   - extern sem_t wejscie2_kierunek;
//   - deklaracje occupant_increment(), occupant_decrement() i lock_queue(), unlock_queue() itd.

/**
 * Pszczoła próbuje wejść do ula, korzystając z jednego z wejść (wejscie1_kierunek / wejscie2_kierunek).
 * Gdy uda się zarezerwować kierunek i semafor `ul_wejscie`, occupant_count++ i wchodzimy.
 */
void hive_entry(int id) {
    int success = 0;

    while (!success && !stop) {
        // Próba wejścia 1
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            // Rezerwujemy "miejsce w ulu"
            if (sem_trywait(ul_wejscie) == 0) {
                occupant_increment();
                printf("\033[0;34mPszczoła %d:  wchodzi do ula wejściem 1.\033[0m\n", id);
                sem_post(&wejscie1_kierunek);
                success = 1;
            } else {
                sem_post(&wejscie1_kierunek);
                printf("\033[0;34mPszczoła %d:  czeka - ul pełny.\033[0m\n", id);
                sleep(1);
            }
        }
        // Próba wejścia 2
        else if (sem_trywait(&wejscie2_kierunek) == 0) {
            if (sem_trywait(ul_wejscie) == 0) {
                occupant_increment();
                printf("\033[0;34mPszczoła %d:  wchodzi do ula wejściem 2.\033[0m\n", id);
                sem_post(&wejscie2_kierunek);
                success = 1;
            } else {
                sem_post(&wejscie2_kierunek);
                printf("\033[0;34mPszczoła %d:  czeka - ul pełny.\033[0m\n", id);
                sleep(1);
            }
        }
        else {
            // Oba wejścia zajęte – poczekaj
            printf("\033[0;33mPszczoła %d:  czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}

/**
 * Pszczoła opuszcza ul – occupant_count--, zwalniamy semafor ul_wejscie
 */
void hive_leave(int id) {
    int success = 0;

    while (!success && !stop) {
        if (sem_trywait(&wejscie1_kierunek) == 0) {
            occupant_decrement();
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d:  wychodzi z ula wejściem 1.\033[0m\n", id);
            sem_post(&wejscie1_kierunek);
            success = 1;
        }
        else if (sem_trywait(&wejscie2_kierunek) == 0) {
            occupant_decrement();
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d:  wychodzi z ula wejściem 2.\033[0m\n", id);
            sem_post(&wejscie2_kierunek);
            success = 1;
        }
        else {
            printf("\033[0;33mPszczoła %d:  czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}

/**
 * Wyświetla stan ula: ile pszczół, ile jaj, ile wolnego miejsca.
 * Zamiast globalnego `capacity`, używamy `eggQueue->capacity`.
 */
void hive_state(void) {
    lock_queue();
    int occupant   = eggQueue->occupant_count; 
    int egg_count  = eggQueue->size;
    int cap        = eggQueue->capacity;
    unlock_queue();

    int bee_count = occupant - egg_count;
    if (bee_count < 0) bee_count = 0;

    int free_space = cap - occupant;
    if (free_space < 0) {
        free_space = 0; // na wszelki wypadek
    }

    if (free_space == 0 && occupant >= cap) {
        printf("\033[0;33mUl jest pełny.\033[0m | Bees: %d, Eggs: %d\n",
               bee_count, egg_count);
    } else {
        printf("\033[0;34mWolna przestrzeń: %d (\033[0mzajęte: %d). Bees: %d, Eggs: %d\n",
               free_space, occupant, bee_count, egg_count);
    }
}

/**
 * Zmiana pojemności ula i dostosowanie semafora `ul_wejscie` do nowej wartości.
 * Zamiast globalnego `capacity` – modyfikujemy `eggQueue->capacity`.
 */
void adjust_hive_capacity(int new_capacity) {
    lock_queue();
    int old_capacity   = eggQueue->capacity;
    eggQueue->capacity = new_capacity;
    unlock_queue();

    int diff = new_capacity - old_capacity;

    if (diff > 0) {
        // Zwiększamy semafor (dodajemy "wolne miejsca")
        for (int i = 0; i < diff; i++) {
            sem_post(ul_wejscie);
        }
        printf("\033[0;35mZwiększanie pojemności ula z %d do %d\033[0m\n",
               old_capacity, new_capacity);
    }
    else if (diff < 0) {
        // Zmniejszamy semafor (usuwamy "wolne miejsca")
        diff = -diff;
        for (int i = 0; i < diff; i++) {
            sem_trywait(ul_wejscie);
        }
        printf("\033[0;35mZmniejszanie pojemności ula z %d do %d\033[0m\n",
               old_capacity, new_capacity);
    }
    else {
        // Bez zmian
        printf("Pojemność ula pozostaje taka sama: %d\n", new_capacity);
    }
}
