


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    int time_in_hive;
    int visits_left;
} Bee;