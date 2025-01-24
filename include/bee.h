#ifndef BEE_H
#define BEE_H


#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;

typedef struct {
    int id;
    int hatch_time;
} Egg;
 
Bee* createBee(int id, int time_in_hive, int visits_left);
void* bee_life(void* arg);

#endif