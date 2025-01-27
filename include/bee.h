#ifndef BEE_H
#define BEE_H
extern int capacity;

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;

 
Bee* createBee(int id, int time_in_hive, int visits_left);
void* bee_life(void* arg);

#endif