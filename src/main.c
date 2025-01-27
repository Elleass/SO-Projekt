#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "bee.h"
#include "queen.h"
#include "egg.h"

// Define the global capacity here (exactly once)
int capacity = 0;

// Global semaphores
sem_t *ul_wejscie;
sem_t wejscie1_kierunek;
sem_t wejscie2_kierunek;

// Global stop flag for graceful termination
volatile sig_atomic_t stop = 0;

// Track initial bees
pthread_t starter_bees[1000];
int num_starter_bees = 0;
int next_bee_id = 0;

EggQueue* eggQueue = NULL;

// PIDs for queen and hatch processes
pid_t queen_pid = -1;
pid_t hatch_pid = -1;

// --------------------------------------------------------------------
// Cleanup function for processes
void cleanup_processes() {
    if (queen_pid > 0) {
        kill(queen_pid, SIGTERM);
        waitpid(queen_pid, NULL, 0);
        printf("Queen process terminated.\n");
    }
    if (hatch_pid > 0) {
        kill(hatch_pid, SIGTERM);
        waitpid(hatch_pid, NULL, 0);
        printf("Hatch process terminated.\n");
    }
}
// --------------------------------------------------------------------
// Cleanup function
void cleanup() {
    if (ul_wejscie) {
        munmap(ul_wejscie, sizeof(sem_t));
    }
    if (eggQueue) {
        munmap(eggQueue, sizeof(EggQueue));
    }
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);
    cleanup_processes();
    printf("Resources cleaned up. Program exiting.\n");
}



// --------------------------------------------------------------------
// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    (void)sig;  // unused parameter
    // Simply set stop to 1 here.
    // Do NOT call pthread_join in a signal handler!
    stop = 1;
    printf("\nSIGINT received. Stopping threads...\n");
}

// --------------------------------------------------------------------
// Main
int main() {
    // Register SIGINT handler
    signal(SIGINT, handle_sigint);

    // Prompt user for capacity
    printf("Podaj maksymalną liczbę pszczół w ulu: ");
    if (scanf("%d", &capacity) != 1 || capacity <= 0) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej większej od zera.\n");
        exit(EXIT_FAILURE);
    }
    printf("Maksymalna liczba pszczół w ulu: %d\n", capacity);

    // Prompt user for starting number of bees
    int start_bees = 0;
    printf("Podaj startową liczbę pszczół (maksymalnie %d): ", capacity / 2);
    if (scanf("%d", &start_bees) != 1 || start_bees <= 0 || start_bees > capacity / 2) {
        fprintf(stderr, 
                "Błąd: Wprowadzono niepoprawne dane. Startowa liczba pszczół musi być liczbą całkowitą z zakresu 1-%d.\n",
                capacity / 2);
        exit(EXIT_FAILURE);
    }
    num_starter_bees = start_bees;

    // Initialize egg queue
eggQueue = initSharedEggQueue();
if (!eggQueue) {
    fprintf(stderr, "Failed to initialize shared EggQueue.\n");
    exit(EXIT_FAILURE);
}
// Debugging: Confirm semaphore value

// Initialize shared semaphore
// Map shared memory for semaphore
ul_wejscie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
if (ul_wejscie == MAP_FAILED) {
    perror("Failed to map shared memory for semaphore");
    exit(EXIT_FAILURE);
}
printf("Shared memory for semaphore successfully mapped.\n");

// Initialize semaphore
if (sem_init(ul_wejscie, 1, capacity) != 0) {
    perror("Failed to initialize semaphore");
    munmap(ul_wejscie, sizeof(sem_t));
    exit(EXIT_FAILURE);
}
printf("Semaphore initialized with value: %d\n", capacity);

// Debug semaphore value
int sem_value;
if (sem_getvalue(ul_wejscie, &sem_value) != 0) {
    perror("Failed to get semaphore value");
    munmap(ul_wejscie, sizeof(sem_t));
    exit(EXIT_FAILURE);
}
printf("Semaphore value after initialization: %d\n", sem_value);


    sem_init(&wejscie1_kierunek, 0, 1);
    sem_init(&wejscie2_kierunek, 0, 1);

    // Create the initial bee threads
  for (int i = 0; i < num_starter_bees; i++) {
    Bee* b = createBee(next_bee_id++, 4, 3);  // Use next_bee_id
    if (pthread_create(&starter_bees[i], NULL, bee_life, b) != 0) {
        fprintf(stderr, "Error: Failed to create thread for Bee ID %d.\n", b->id);
        exit(EXIT_FAILURE);
    }
}


   // Create the queen process
    queen_pid = fork();
    if (queen_pid == 0) {
        queen_process(eggQueue);
    } else if (queen_pid < 0) {
        perror("Failed to create queen process");
        exit(EXIT_FAILURE);
    }

    // Create the hatch process
    hatch_pid = fork();
    if (hatch_pid == 0) {
        hatch_eggs(eggQueue);
    } else if (hatch_pid < 0) {
        perror("Failed to create hatch process");
        exit(EXIT_FAILURE);
    }

    // Main process loop
    while (!stop) {
        sleep(1);
    }

    // 1. Join the starter bee threads
    for (int i = 0; i < num_starter_bees; i++) {
        pthread_join(starter_bees[i], NULL);
    }

    // 2. Join the queen and hatch threads
waitpid(queen_pid, NULL, 0);
waitpid(hatch_pid, NULL, 0);


    // Cleanup semaphores, etc.
    cleanup();
    return 0;
}
