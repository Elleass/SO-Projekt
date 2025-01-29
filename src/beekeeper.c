
#include "beekeper.h"

/********************************************
 * Beekeeper thread
 ********************************************/
void* beekeeper(void* arg) {
    (void)arg; // unused
    while (!stop) {
        int signal_code;
        printf("Czekam na sygnał (4 = dodaj ramki, 5 = usuń ramki): ");
        if (scanf("%d", &signal_code) != 1) {
            fprintf(stderr, "Niepoprawne dane. Spróbuj ponownie.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {} // flush
            continue;
        }

        if (signal_code == 4) {
            int new_capacity = capacity * 2;
            if (new_capacity > 2 * capacity) { 
                // (In principle you might want to clamp to 2*N if that's your limit, 
                //  but for now we just follow the user request.)
            }
            adjust_hive_capacity(new_capacity);
            capacity = new_capacity;
        } else if (signal_code == 5) {
            int new_capacity = capacity / 2;
            if (new_capacity < 1) {
                printf("Minimalna pojemność ula to 1. Pomiń lub ustaw większą liczbę.\n");
                continue;
            }
            adjust_hive_capacity(new_capacity);
            capacity = new_capacity;
        } else {
            printf("Nieznany sygnał.\n");
        }
    }
    return NULL;
}