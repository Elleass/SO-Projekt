/*******************************************************
 * hive.c
 *
 * Implementacja funkcji sterujących "ulem":
 *  - hive_entry() / hive_leave() – wchodzenie/wychodzenie pszczół
 *  - hive_state() – wyświetlanie stanu ula
 *  - adjust_hive_capacity() – zmiana pojemności ula i dostosowanie semafora
 *******************************************************/

#include "hive.h"
#include "egg.h"
#include "error_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



#define DEBUG_SEM 1  // set to 1 to enable

int my_sem_trywait(sem_t* s, const char* semname, int bee_id) {
    int rv = sem_trywait(s);
#if DEBUG_SEM
    if (rv == 0) {
        printf("[DEBUG] Bee %d acquired %s\n", bee_id, semname);
    } else {
        // typically EAGAIN if zero
    }
#endif
    return rv;
}

void my_sem_post(sem_t* s, const char* semname, int bee_id) {
    sem_post(s);
#if DEBUG_SEM
    printf("[DEBUG] Bee %d posted %s\n", bee_id, semname);
#endif
}


/**
 * hive_entry
 *  - Pszczoła próbuje wejść do ula.
 *  - Korzysta z jednego z dwóch semaforów wejść (wejscie1_kierunek / wejscie2_kierunek).
 *  - Jeśli ul jest pełny, czeka.
 */
void hive_entry(int id)
{
    int success = 0;

    while (!success && !stop)
    {
        // Próba wejścia 1
        if (my_sem_trywait(wejscie1_kierunek_ptr, "wejscie1_kierunek", id) == 0)
        {
            if (sem_trywait(ul_wejscie) == 0)
            {
                occupant_increment();  
                printf("\033[0;34mPszczoła %d: wchodzi do ula wejściem 1.\033[0m\n", id);
                my_sem_post(wejscie1_kierunek_ptr, "wejscie1_kierunek", id);
                success = 1;
            }
            else
            {
                my_sem_post(wejscie1_kierunek_ptr, "wejscie1_kierunek", id);
                printf("\033[0;34mPszczoła %d: czeka - ul pełny.\033[0m\n", id);
                sleep(1);
            }
        }
        else if (my_sem_trywait(wejscie2_kierunek_ptr, "wejscie2_kierunek", id) == 0)
        {
            if (sem_trywait(ul_wejscie) == 0)
            {
                occupant_increment();
                printf("\033[0;34mPszczoła %d: wchodzi do ula wejściem 2.\033[0m\n", id);
                my_sem_post(wejscie2_kierunek_ptr, "wejscie2_kierunek", id);
                success = 1;
            }
            else
            {
                my_sem_post(wejscie2_kierunek_ptr, "wejscie2_kierunek", id);
                printf("\033[0;34mPszczoła %d: czeka - ul pełny.\033[0m\n", id);
                sleep(1);
            }
        }
        else
        {
            printf("\033[0;33mPszczoła %d: czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}

/**
 * hive_leave
 *  - Pszczoła opuszcza ul (occupant_decrement).
 *  - Zwalnia semafor ul_wejscie.
 */
void hive_leave(int id)
{
    int success = 0;

    while (!success && !stop)
    {
        if (my_sem_trywait(wejscie1_kierunek_ptr, "wejscie1_kierunek", id) == 0)
        {
            occupant_decrement();
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d: wychodzi z ula wejściem 1.\033[0m\n", id);
            my_sem_post(wejscie1_kierunek_ptr, "wejscie1_kierunek", id);
            success = 1;
        }
        else if (my_sem_trywait(wejscie2_kierunek_ptr, "wejscie2_kierunek", id) == 0)
        {
            occupant_decrement();
            sem_post(ul_wejscie);
            printf("\033[0;34mPszczoła %d: wychodzi z ula wejściem 2.\033[0m\n", id);
            my_sem_post(wejscie2_kierunek_ptr, "wejscie2_kierunek", id);
            success = 1;
        }
        else
        {
            printf("\033[0;33mPszczoła %d: czeka, oba wejścia są zajęte.\033[0m\n", id);
            sleep(1);
        }
    }
}

/**
 * hive_state
 *  - Wyświetla stan ula: 
 *    -> ile pszczół (bee_count = occupant_count - liczbę jaj)
 *    -> ile jaj 
 *    -> ile wolnego miejsca
 */
void hive_state(void)
{
    lock_queue();
    int occupant  = eggQueue->occupant_count;
    int egg_count = eggQueue->size;
    int cap       = eggQueue->capacity;
    unlock_queue();

    int bee_count = occupant - egg_count;
    if (bee_count < 0) bee_count = 0;

    int free_space = cap - occupant;
    if (free_space < 0) free_space = 0;

    if (free_space == 0 && occupant >= cap)
    {
        printf("\033[0;33mUl jest pełny.\033[0m | Bees: %d, Eggs: %d\n",
               bee_count, egg_count);
    }
    else
    {
        printf("\033[0;34mWolna przestrzeń: %d (\033[0mzajęte: %d). Bees: %d, Eggs: %d\n",
               free_space, occupant, bee_count, egg_count);
    }
}

/**
 * adjust_hive_capacity
 *  - Zmienia pojemność ula (eggQueue->capacity).
 *  - Dostosowuje semafor ul_wejscie (dodaje/usuwa tokeny).
 */
void adjust_hive_capacity(int new_capacity)
{
    lock_queue();
    int old_capacity   = eggQueue->capacity;
    eggQueue->capacity = new_capacity;
    unlock_queue();

    int diff = new_capacity - old_capacity;

    if (diff > 0)
    {
        for (int i = 0; i < diff; i++)
        {
            sem_post(ul_wejscie);
        }
        printf("\033[0;35mZwiększanie pojemności ula z %d do %d\033[0m\n",
               old_capacity, new_capacity);
    }
    else if (diff < 0)
    {
        diff = -diff;
        for (int i = 0; i < diff; i++)
        {
            sem_trywait(ul_wejscie);
        }
        printf("\033[0;35mZmniejszanie pojemności ula z %d do %d\033[0m\n",
               old_capacity, new_capacity);
    }
    else
    {
        printf("Pojemność ula pozostaje taka sama: %d\n", new_capacity);
    }
}
