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
#include "beekeper.h"
#include "error_handling.h"

// Define the global capacity here (exactly once)
int capacity = 0;
int next_bee_id;
int N;

// Global semaphores
sem_t *ul_wejscie;
sem_t wejscie1_kierunek;
sem_t wejscie2_kierunek;

// Global stop flag for graceful termination
volatile sig_atomic_t stop = 0;

// Track initial bees
pthread_t starter_bees;
pthread_t beekeeper_thread;



EggQueue *eggQueue = NULL;
// PIDs for queen and hatch processes
pid_t queen_pid = -1;
pid_t hatch_pid = -1;


// 4) Cleanup
/************************************************
 * Cleanup function to gracefully stop the program
 ************************************************/
void cleanup() {
    printf("Starting cleanup...\n");

    // Terminate queen process
    if (queen_pid > 0) {
        printf("Terminating queen process (PID: %d)...\n", queen_pid);
        kill(queen_pid, SIGTERM);
        waitpid(queen_pid, NULL, 0);
        printf("Queen process terminated.\n");
    }

    // Terminate hatch process
    if (hatch_pid > 0) {
        printf("Terminating hatch process (PID: %d)...\n", hatch_pid);
        kill(hatch_pid, SIGTERM);
        waitpid(hatch_pid, NULL, 0);
        printf("Hatch process terminated.\n");
    }

    // Cancel beekeeper thread
    if (beekeeper_thread) {
        printf("Canceling beekeeper thread...\n");
        pthread_cancel(beekeeper_thread);
        pthread_join(beekeeper_thread, NULL);
        printf("Beekeeper thread terminated.\n");
    }

    // Cancel and join all bee threads
    pthread_mutex_lock(&bee_list_mutex);
    printf("Canceling %d bee threads...\n", bee_thread_count);
    for (int i = 0; i < bee_thread_count; i++) {
        pthread_cancel(bee_threads[i]);
    }
    for (int i = 0; i < bee_thread_count; i++) {
        pthread_join(bee_threads[i], NULL);
    }
    free(bee_threads);
    bee_threads = NULL;
    bee_thread_capacity = 0;
    bee_thread_count = 0;
    pthread_mutex_unlock(&bee_list_mutex);
    printf("Bee threads cleaned up.\n");

    // Destroy semaphores
    printf("Destroying semaphores...\n");
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);
    if (ul_wejscie != MAP_FAILED) {
        sem_destroy(ul_wejscie);
        munmap(ul_wejscie, sizeof(sem_t));
    }
    printf("Semaphores destroyed.\n");

    // Destroy egg queue
    if (eggQueue) {
        printf("Destroying egg queue...\n");
        destroySharedEggQueue(eggQueue); // Assuming destroySharedEggQueue() exists
        eggQueue = NULL;
        printf("Egg queue destroyed.\n");
    }

    printf("Cleanup completed. Exiting.\n");
}



/********************************************
 * Signal handler for SIGINT
 ********************************************/
void handle_sigint(int sig) {
    (void)sig; // Unused parameter
    static int already_handled = 0; // Ensure only the first signal is processed

    if (__sync_lock_test_and_set(&already_handled, 1) == 0) { // Atomic check-and-set
        stop = 1;
        printf("\nSIGINT received. Stopping...\n");
    }
}
/********************************************
 * MAIN
 ********************************************/
int main() {
    // Register SIGINT handler
    struct sigaction sa;
sa.sa_handler = handle_sigint;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;

if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Error setting up SIGINT handler");
    exit(EXIT_FAILURE);
}

    // Prompt user for capacity

    printf("Podaj calkowitą liczbę pszczol w roju (N) :  ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej!! > 0.\n");
        exit(EXIT_FAILURE);
    }
    


    // Prompt user for starting number of bees
       int  P;
    printf("Podaj maksymalną liczbę pszczol w roju (P), gdzie (P) < N/2: ");
    if (scanf("%d", &P) != 1 || P <= 0 || P >= N/2) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. P musi być dodatnie i < N/2!!.\n");
        exit(EXIT_FAILURE);
    }
    capacity = P;
        printf("Maksymalna liczba pszczół w ulu: %d\n", capacity);
    int num_starter_bees = N;

    // Initialize shared EggQueue (including occupant_count)
    Error err = initSharedEggQueue(&eggQueue);
    if (err.code != ERR_SUCCESS) {
        handle_error(err);
        exit(EXIT_FAILURE);
    }

    // Allocate shared memory for ul_wejscie (the main hive semaphore for blocking)
    ul_wejscie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ul_wejscie == MAP_FAILED) {
        perror("Failed to map ul_wejscie in shared memory");
        exit(EXIT_FAILURE);
    }

    // Initialize the shared semaphore (value = capacity)
    if (sem_init(ul_wejscie, 1, capacity) != 0) {
        perror("Failed to sem_init ul_wejscie");
        munmap(ul_wejscie, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    // Initialize local semaphores for entry direction
    sem_init(&wejscie1_kierunek, 0, 1);
    sem_init(&wejscie2_kierunek, 0, 1);

    // Create initial bees
    for (int i = 0; i < num_starter_bees; i++) {
    Bee* b = createBee(next_bee_id++, 4, 3);
    if (!b) {
        fprintf(stderr, "Error: Bee creation failed for index %d.\n", i);
        continue;
    }

    create_a_bee(b); // Use the helper function to create and register the bee
}

    // Create queen process
    queen_pid = fork();
   if (queen_pid == 0) {
    // Child: Queen
    struct sigaction ignore_sa;
    ignore_sa.sa_handler = SIG_IGN;
    sigemptyset(&ignore_sa.sa_mask);
    ignore_sa.sa_flags = 0;
    sigaction(SIGINT, &ignore_sa, NULL);

    queen_process(eggQueue);
    _exit(0);
    } else if (queen_pid < 0) {
        perror("Failed to create queen process");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Create hatch process
    hatch_pid = fork();
    if (hatch_pid == 0) {
    struct sigaction ignore_sa;
    ignore_sa.sa_handler = SIG_IGN;
    sigemptyset(&ignore_sa.sa_mask);
    ignore_sa.sa_flags = 0;
    sigaction(SIGINT, &ignore_sa, NULL);

    hatch_eggs(eggQueue);
    _exit(0);
} else if (hatch_pid < 0) {
        perror("Failed to create hatch process");
        cleanup();
        exit(EXIT_FAILURE);
    }

    

    // Create beekeeper thread
    pthread_t beekeeper_thread;
    if (pthread_create(&beekeeper_thread, NULL, beekeeper, NULL) != 0) {
        perror("Błąd: Nie udało się utworzyć wątku pszczelarza.");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // We'll *not* join the beekeeper thread so the program remains interactive
    // (for signals 4,5). Similarly, queen/hatch processes run until Ctrl+C.

    printf("Program is running. Press CTRL+C to stop.\n");
    while (!stop) {
        sleep(1);
    }

    // final cleanup
    cleanup();
    return 0;
}