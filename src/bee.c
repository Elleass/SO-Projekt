
#include "../include/bee.h"


Bee* createBee(int id, int time_in_hive, int visits_left){
    Bee* new_bee = (Bee*)malloc(sizeof(Bee));
    if (!new_bee) {
        perror("Nie udało się przydzielić pamięci dla pszczoły");
        exit(EXIT_FAILURE);
    }
    new_bee->id = id;
    new_bee->time_in_hive = time_in_hive;
    new_bee->visits_left = visits_left;
    return new_bee;
}

void* bee_life(void* arg) {
    Bee* bee = (Bee*)arg;

    printf("Pszczoła %d startuje w wątku.\n", bee->id);

    while (bee->visits_left > 0) {
        printf("Pszczoła %d: wchodzi do ula na %d sekund.\n", bee->id, bee->time_in_hive);
        // symulacja bycia w ulu
        sleep(bee->time_in_hive);

        bee->visits_left--;
        printf("Pszczoła %d: wychodzi, wizyt pozostało: %d\n", bee->id, bee->visits_left);

        // symulacja czasu poza ulem
        sleep(1);
    }

    printf("Pszczoła %d kończy życie.\n", bee->id);
    free(bee);
    return NULL;
}