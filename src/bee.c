
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