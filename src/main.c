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

EggQueue *eggQueue = NULL;
// PIDs for queen and hatch processes
pid_t queen_pid = -1;
pid_t hatch_pid = -1;

// --------------------------------------------------------------------

/********************************************
 * Cleanup routines
 ********************************************/
void cleanup_processes() {
    // Kill queen if alive
    if (queen_pid > 0) {
        kill(queen_pid, SIGTERM);
        waitpid(queen_pid, NULL, 0);
        printf("Queen process terminated.\n");
    }
    // Kill hatch if alive
    if (hatch_pid > 0) {
        kill(hatch_pid, SIGTERM);
        waitpid(hatch_pid, NULL, 0);
        printf("Hatch process terminated.\n");
    }
}

void cleanup() {
    // Terminate child processes
    cleanup_processes();

    // Destroy semaphores
    if (ul_wejscie) {
        sem_destroy(ul_wejscie);  // unnamed semaphore stored in shared memory
        munmap(ul_wejscie, sizeof(sem_t));
        ul_wejscie = NULL;
    }
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);

    // Destroy egg queue
    destroySharedEggQueue();

    printf("Resources cleaned up. Program exiting.\n");
}

/********************************************
 * Signal handler for SIGINT
 ********************************************/
void handle_sigint(int sig) {
    (void)sig;
    stop = 1;
    printf("\nSIGINT received. Stopping...\n");
    // Once main sees stop == 1, it calls cleanup() then exits,
    // thus terminating all threads and child processes immediately.
}

/********************************************
 * MAIN
 ********************************************/
int main() {
    // Register SIGINT handler
    signal(SIGINT, handle_sigint);

    // Prompt user for capacity
    printf("Podaj maksymalną liczbę pszczół w ulu: ");
    if (scanf("%d", &capacity) != 1 || capacity <= 0) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej > 0.\n");
        exit(EXIT_FAILURE);
    }
    printf("Maksymalna liczba pszczół w ulu: %d\n", capacity);

    // Prompt user for starting number of bees
    int start_bees = 0;
    printf("Podaj startową liczbę pszczół (maksymalnie %d): ", capacity / 2);
    if (scanf("%d", &start_bees) != 1 || start_bees <= 0 || start_bees > capacity / 2) {
        fprintf(stderr,
                "Błąd: Wprowadzono niepoprawne dane. Musi być 1-%d.\n",
                capacity / 2);
        exit(EXIT_FAILURE);
    }
    int num_starter_bees = start_bees;

    // Initialize shared EggQueue
    Error err = initSharedEggQueue(&eggQueue);
    if (err.code != ERR_SUCCESS) {
        handle_error(err);
        exit(EXIT_FAILURE);
    }

    // Allocate shared memory for ul_wejscie (the main hive semaphore)
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
    pthread_t starter_bees_threads[1000];
    for (int i = 0; i < num_starter_bees; i++) {
        Bee* b = createBee(next_bee_id++, 4, 3);
        if (!b) {
            fprintf(stderr, "Error: Bee creation failed for index %d.\n", i);
            continue;
        }
        if (pthread_create(&starter_bees_threads[i], NULL, bee_life, b) != 0) {
            fprintf(stderr, "Error: Failed to create thread for Bee ID %d.\n", b->id);
            free(b);
            continue;
        }
    }

    // Create queen process
    queen_pid = fork();
    if (queen_pid == 0) {
        // Child: queen process
        queen_process(eggQueue);
        _exit(0); // never reached unless error
    } else if (queen_pid < 0) {
        perror("Failed to create queen process");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Create hatch process
    hatch_pid = fork();
    if (hatch_pid == 0) {
        // Child: hatch process
        hatch_eggs(eggQueue);
        _exit(0); // never reached unless error
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

    // Join the starter bees so main won't exit prematurely
    for (int i = 0; i < num_starter_bees; i++) {
        pthread_join(starter_bees_threads[i], NULL);
    }

    // We'll *not* join the beekeeper thread so the program remains interactive
    // and doesn't exit until we get Ctrl+C. The queen & hatch processes also run.

    printf("Program is running. Press CTRL+C to stop.\n");
    // Wait until user presses Ctrl+C => handle_sigint sets stop=1
    while (!stop) {
        sleep(1);
    }

    // Once stop=1, we proceed to cleanup:
    cleanup();

    // Exiting main kills any leftover threads automatically.
    return 0;
}