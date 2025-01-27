
#include "beekeper.h"
void* beekeeper(void* arg) {
    while (!stop) {
        int signal;
        printf("Czekam na sygnał (4 = dodaj ramki, 5 = usuń ramki): ");
        scanf("%d", &signal);

        if (signal == 4) {
            int new_capacity = capacity * 2;
            adjust_hive_capacity(new_capacity);
            capacity = new_capacity;
        } else if (signal == 5) {
            int new_capacity = capacity / 2;
            adjust_hive_capacity(new_capacity);
            capacity = new_capacity;
        } else {
            printf("Nieznany sygnał.\n");
        }
    }
    return NULL;
}
