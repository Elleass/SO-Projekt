#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>

// Nagłówki projektowe
#include "bee.h"
#include "queen.h"
#include "egg.h"
#include "beekeper.h"
#include "error_handling.h"

// ------------------ GLOBALNE ZMIENNE/WSKAŹNIKI ------------------

// Usuwamy globalną zmienną "int capacity;" – teraz będzie w EggQueue.
int next_bee_id;      // do nadawania unikalnych ID nowym pszczołom
int N;                // liczba pszczół docelowo (wczytywana z klawiatury)

// Semafory globalne
sem_t *ul_wejscie;             // semafor ograniczający liczbę osobników w ulu
sem_t wejscie1_kierunek;       // semafor kierunku wejścia 1
sem_t wejscie2_kierunek;       // semafor kierunku wejścia 2

// Flaga stopu (dla wszystkich procesów/wątków)
volatile sig_atomic_t stop = 0;

// Zasoby dla wątków pszczół
pthread_t *bee_threads = NULL;
int bee_thread_count = 0;
int bee_thread_capacity = 0;
pthread_mutex_t bee_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Kolejka jaj w pamięci współdzielonej (zawiera także occupant_count i capacity)
EggQueue *eggQueue = NULL;

// PID-y dla królowej i procesu wylęgania
pid_t queen_pid = -1;
pid_t hatch_pid = -1;

// Wątek pszczelarza
pthread_t beekeeper_thread;


// ------------------ FUNKCJE POMOCNICZE (cleanup, sygnały) ------------------

/************************************************
 * Funkcja sprzątająca (cleanup) – wywoływana przy stop=1
 ************************************************/
void cleanup() {
    printf("Starting cleanup...\n");

    // 1) Zatrzymanie procesów queen i hatch
    if (queen_pid > 0) {
        printf("Terminating queen process (PID: %d)...\n", queen_pid);
        kill(queen_pid, SIGTERM);
        waitpid(queen_pid, NULL, 0);
        printf("Queen process terminated.\n");
    }

    if (hatch_pid > 0) {
        printf("Terminating hatch process (PID: %d)...\n", hatch_pid);
        kill(hatch_pid, SIGTERM);
        waitpid(hatch_pid, NULL, 0);
        printf("Hatch process terminated.\n");
    }

    // 2) Zatrzymanie wątku pszczelarza
    if (beekeeper_thread) {
        printf("Canceling beekeeper thread...\n");
        pthread_cancel(beekeeper_thread);
        pthread_join(beekeeper_thread, NULL);
        printf("Beekeeper thread terminated.\n");
    }

    // 3) Zatrzymanie wszystkich wątków pszczół
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

    // 4) Usunięcie semaforów
    printf("Destroying semaphores...\n");
    sem_destroy(&wejscie1_kierunek);
    sem_destroy(&wejscie2_kierunek);

    if (ul_wejscie != MAP_FAILED) {
        sem_destroy(ul_wejscie);
        munmap(ul_wejscie, sizeof(sem_t));
    }
    printf("Semaphores destroyed.\n");

    // 5) Zwolnienie pamięci współdzielonej dla EggQueue
    if (eggQueue) {
        printf("Destroying egg queue...\n");
        destroySharedEggQueue();  // niszczy segment i semafor SysV
        eggQueue = NULL;
        printf("Egg queue destroyed.\n");
    }

    printf("Cleanup completed. Exiting.\n");
}


/********************************************
 * Obsługa sygnału SIGINT (CTRL + C)
 ********************************************/
void handle_sigint(int sig) {
    (void)sig; // niewykorzystany parametr
    static int already_handled = 0; 

    // Atomicznie sprawdzamy, czy już obsługiwano ten sygnał
    if (__sync_lock_test_and_set(&already_handled, 1) == 0) {
        stop = 1;
        printf("\nSIGINT received. Stopping...\n");
    }
}


/********************************************
 * MAIN
 ********************************************/
int main() {
    // Rejestracja handlera SIGINT
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // 1) Wczytanie wartości N
    printf("Podaj całkowitą liczbę pszczół w roju (N) :  ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej > 0.\n");
        exit(EXIT_FAILURE);
    }

    // 2) Wczytanie wartości P – początkowa pojemność ula (musi być < N/2)
    int P;
    printf("Podaj maksymalną liczbę pszczół w ulu (P), gdzie P < N/2: ");
    if (scanf("%d", &P) != 1 || P <= 0 || P >= N/2) {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. P musi być dodatnie i < N/2.\n");
        exit(EXIT_FAILURE);
    }
    printf("Maksymalna liczba pszczół w ulu: %d\n", P);

    // (dla testów) liczba pszczół startowych = N
    int num_starter_bees = N;

    // 3) Inicjalizacja pamięci współdzielonej i EggQueue
    Error err = initSharedEggQueue(&eggQueue);
    if (err.code != ERR_SUCCESS) {
        handle_error(err);
        exit(EXIT_FAILURE);
    }
    // Ustawiamy początkową pojemność ula w pamięci współdzielonej
    eggQueue->capacity = P;

    // 4) Alokacja pamięci dla semafora ul_wejscie
    ul_wejscie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ul_wejscie == MAP_FAILED) {
        perror("Failed to map ul_wejscie in shared memory");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semafora ul_wejscie wartością = P (pojemność)
    if (sem_init(ul_wejscie, 1, P) != 0) {
        perror("Failed to sem_init ul_wejscie");
        munmap(ul_wejscie, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semaforów wejść (kierunek)
    sem_init(&wejscie1_kierunek, 0, 1);
    sem_init(&wejscie2_kierunek, 0, 1);


        // 6) Uruchomienie procesu "queen"
    queen_pid = fork();
    if (queen_pid == 0) {
        // Proces potomny – królowa
        // Ignorujemy SIGINT w dziecku, żeby go nie ubić zbyt szybko
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

     // 7) Uruchomienie procesu "hatch" (wylęganie jaj)
    hatch_pid = fork();
    if (hatch_pid == 0) {
        // Proces potomny – hatch
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

    
    // 8) Uruchomienie wątku pszczelarza (odbiera sygnały SIGUSR1/SIGUSR2)
    if (pthread_create(&beekeeper_thread, NULL, beekeeper, NULL) != 0) {
        perror("Błąd: Nie udało się utworzyć wątku pszczelarza.");
        cleanup();
        exit(EXIT_FAILURE);
    }


    // 5) Utworzenie wątków startowych pszczół
    for (int i = 0; i < num_starter_bees; i++) {
        Bee* b = createBee(next_bee_id++, 4, 3);
        if (!b) {
            fprintf(stderr, "Error: Bee creation failed for index %d.\n", i);
            continue;
        }
        create_a_bee(b);
    }



   

    // 9) Pętla główna – czekamy do momentu "stop"
    printf("Ctrl + C aby zatrzymac program\n");
    while (!stop) {
        sleep(1);
    }

    // 10) Sprzątanie
    cleanup();
    return 0;
}
