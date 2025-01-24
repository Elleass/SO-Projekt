#include "queen.h"


void queen_process(EggQueue* eggQueue) {
    int egg_id = 0;
    while(1) {
        sleep(5);
        Egg new_egg = {egg_id++, 5};
        if(enqueue(eggQueue, new_egg) == 0) {
            printf("Queen: Laid egg ID %d , hatching in %d seconds \n ", new_egg.id, new_egg.hatch_time);
        }else {
            printf("Queen: Egg  queue is full, cannot lay more eggs");
        }
    }
}