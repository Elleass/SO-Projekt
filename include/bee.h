#ifndef BEE_H
#define BEE_H


#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;

Bee* createBee(int id, int time_in_hive, int visits_left);

#endif