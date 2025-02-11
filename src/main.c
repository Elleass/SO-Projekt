/*******************************************************
 * main.c 
 *
 * Główny punkt startowy programu. Zarządza:
 *  - Inicjalizacją pamięci współdzielonej i semaforów
 *  - Tworzeniem wątków pszczół startowych
 *  - Uruchomieniem procesów queen (składanie jaj) i hatch (wylęganie)
 *  - Uruchomieniem wątku pszczelarza (beekeeper)
 *  - Obsługą sygnału SIGINT (Ctrl+C)
 *  - Sprzątaniem (cleanup)
 *******************************************************/

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
#include "beekeeper.h"
#include "error_handling.h"
#include "cleanup.h"
#include "hive.h"

// ------------------ GLOBALNE ZMIENNE/WSPÓŁDZIELONE ------------------

// Liczba pszczół docelowo (wczytywana z klawiatury)
int N = 0;

// Kolejka jaj w pamięci współdzielonej (zawiera occupant_count i capacity)
EggQueue *eggQueue = NULL;

// Dla unikalnych ID pszczół (inkrementowane przy każdym tworzeniu nowej)
int next_bee_id = 0;

// Zasoby wątków pszczół
pthread_t *bee_threads = NULL;
int bee_thread_count = 0;
int bee_thread_capacity = 0;
pthread_mutex_t bee_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Semafory globalne (dla ula)
sem_t *ul_wejscie = NULL;       // ogranicza liczbę osobników w ulu
sem_t *wejscie1_kierunek_ptr;
sem_t *wejscie2_kierunek_ptr;

// Flaga stopu (dla wszystkich procesów i wątków)
volatile sig_atomic_t stop = 0;

pid_t queen_pid = -1;
pid_t hatch_pid = -1;

pthread_t beekeeper_thread;



/**
 * Obsługa sygnału SIGINT (np. Ctrl + C). 
 * Ustawia globalną flagę stop, co w efekcie zatrzymuje program.
 */
static void handle_sigint(int sig)
{
    (void)sig; // niewykorzystany parametr
    static int already_handled = 0;

    // Atomicznie sprawdzamy, czy już obsłużyliśmy SIGINT
    if (__sync_lock_test_and_set(&already_handled, 1) == 0)
    {
        stop = 1;
        printf("\nSIGINT received. Stopping...\n");
    }
}


// ----------------------------------------------------------
// Funkcja main
// ----------------------------------------------------------
int main(void)
{
    // Rejestracja handlera SIGINT
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }

    //Wczytanie wartości N
    printf("Podaj całkowitą liczbę pszczół w roju (N): ");
    if (scanf("%d", &N) != 1 || N <= 0)
    {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. Oczekiwano liczby całkowitej > 0.\n");
        exit(EXIT_FAILURE);
    }

    //Wczytanie wartości P (początkowa pojemność ula, P < N/2)
    int P = 0;
    printf("Podaj maksymalną liczbę pszczół w ulu (P), gdzie P < N/2: ");
    if (scanf("%d", &P) != 1 || P <= 0 || P >= N / 2)
    {
        fprintf(stderr, "Błąd: Wprowadzono niepoprawne dane. P musi być dodatnie i < N/2.\n");
        exit(EXIT_FAILURE);
    }
    printf("Maksymalna liczba pszczół w ulu: %d\n", P);

    int num_starter_bees = N;

    Error err = initSharedEggQueue(&eggQueue);
    if (err.code != ERR_SUCCESS)
    {
        handle_error(err);
        exit(EXIT_FAILURE);
    }
    //Ustawiamy początkową pojemność ula
    eggQueue->capacity = P;

    //Alokacja pamięci dla semafora ul_wejscie (z mapowaniem anon.)
    ul_wejscie = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ul_wejscie == MAP_FAILED)
    {
        perror("Failed to map ul_wejscie in shared memory");
        exit(EXIT_FAILURE);
    }
    //Inicjalizacja semafora ul_wejscie wartością = P (pojemność)
    if (sem_init(ul_wejscie, 1, P) != 0)
    {
        perror("Failed to sem_init ul_wejscie");
        munmap(ul_wejscie, sizeof(sem_t));
        exit(EXIT_FAILURE);
    }

    //Inicjalizacja semaforów wejściowych (kierunków)
    wejscie1_kierunek_ptr = mmap(NULL, sizeof(sem_t),
    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

wejscie2_kierunek_ptr = mmap(NULL, sizeof(sem_t),
    PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

if (wejscie1_kierunek_ptr == MAP_FAILED || wejscie2_kierunek_ptr == MAP_FAILED) {
    perror("mmap for wejscie pointers");
    exit(1);
}

sem_init(wejscie1_kierunek_ptr, 1, 1); // pshared=1
sem_init(wejscie2_kierunek_ptr, 1, 1); // pshared=1

    //Tworzenie wątków startowych pszczół
    for (int i = 0; i < num_starter_bees; i++)
    {
        Bee *b = createBee(next_bee_id++, 4, 1); // np. time_in_hive=4, visits_left=1
        if (!b)
        {
            fprintf(stderr, "Error: Bee creation failed for index %d.\n", i);
            continue;
        }
        create_a_bee(b);
    }


    queen_pid = fork();
    if (queen_pid == 0)
    {
        struct sigaction ignore_sa;
        ignore_sa.sa_handler = SIG_IGN;
        sigemptyset(&ignore_sa.sa_mask);
        ignore_sa.sa_flags = 0;
        sigaction(SIGINT, &ignore_sa, NULL);

        queen_process(eggQueue);
        _exit(0);
    }
    else if (queen_pid < 0)
    {
        perror("Failed to create queen process");
        cleanup();
        exit(EXIT_FAILURE);
    }

    hatch_pid = fork();
    if (hatch_pid == 0)
    {
        struct sigaction ignore_sa;
        ignore_sa.sa_handler = SIG_IGN;
        sigemptyset(&ignore_sa.sa_mask);
        ignore_sa.sa_flags = 0;
        sigaction(SIGINT, &ignore_sa, NULL);

        hatch_eggs(eggQueue);
        _exit(0);
    }
    else if (hatch_pid < 0)
    {
        perror("Failed to create hatch process");
        cleanup();
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&beekeeper_thread, NULL, beekeeper, NULL) != 0)
    {
        perror("Błąd: Nie udało się utworzyć wątku pszczelarza.");
        cleanup();
        exit(EXIT_FAILURE);
    }

    //Pętla główna – czekamy aż stop == 1 (np. SIGINT)
    printf("Naciśnij Ctrl + C, aby zatrzymać program...\n");
    while (!stop)
    {
        sleep(1);
    }
    
    cleanup();
    return 0;
}
