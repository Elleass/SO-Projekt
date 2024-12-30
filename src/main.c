#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/bee.h"


int main() {
    int capacity = 0;
    int start_bees = 0;
    printf("Podaj maksymalną liczbę pszczół w ulu: ");
    scanf("%d", &capacity);
    printf("Maksymalna liczba pszczół w ulu: %d\n", capacity);
    printf("Podaj startową liczbę pszczół (maksymalnie %d)\n", (capacity/2));
    scanf("%d", &start_bees);
    if(start_bees > capacity/2)
        perror("Za duża liczba pszczół");        
        exit(EXIT_FAILURE);
 
    return 0;
}