/*******************************************************
 * beekeeper.c
 *
 * Implementacja wątku pszczelarza, który:
 *  - Obsługuje sygnały SIGUSR1 i SIGUSR2
 *  - Zwiększa/zmniejsza pojemność ula (wywołuje adjust_hive_capacity)
 *******************************************************/

 #include <stdio.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <unistd.h>
 #include <pthread.h>
 
 #include "beekeeper.h"
 #include "hive.h"
 #include "egg.h"
 #include "error_handling.h"
 
 // Zewnętrzna flaga zatrzymania (wspólna z main.c)
 extern volatile sig_atomic_t stop;
 
 // Zmienna N (liczba pszczół w roju), zadeklarowana w main.c
 extern int N;
 
 /** 
  * Prototypy lokalnych funkcji obsługujących sygnały:
  *  - handle_sigusr1
  *  - handle_sigusr2
  */
 static void handle_sigusr1(int signo);
 static void handle_sigusr2(int signo);
 
 /**
  * Funkcja wątku pszczelarza (beekeeper).
  *  - Rejestruje obsługę sygnałów SIGUSR1 i SIGUSR2
  *  - Następnie czeka na sygnały (poprzez `pause()` w pętli).
  */
 void* beekeeper(void* arg)
 {
     // Ustawiamy tryb anulowania wątku na DEFERRED (domyślny),
     // czyli pthread_cancel zadziała dopiero przy punkcie sprawdzania (np. pause()).
     pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
     pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
 
     // Obsługa SIGUSR1 (powiększ ul)
     struct sigaction sa1;
     sa1.sa_handler = handle_sigusr1;
     sigemptyset(&sa1.sa_mask);
     sa1.sa_flags = 0;
     if (sigaction(SIGUSR1, &sa1, NULL) < 0)
     {
         perror("sigaction(SIGUSR1) error");
         pthread_exit(NULL);
     }
 
     // Obsługa SIGUSR2 (zmniejsz ul)
     struct sigaction sa2;
     sa2.sa_handler = handle_sigusr2;
     sigemptyset(&sa2.sa_mask);
     sa2.sa_flags = 0;
     if (sigaction(SIGUSR2, &sa2, NULL) < 0)
     {
         perror("sigaction(SIGUSR2) error");
         pthread_exit(NULL);
     }
 
     printf("[Beekeeper] Oczekiwanie na sygnały:\n");
     printf("  - SIGUSR1 (powiększ ul)\n");
     printf("  - SIGUSR2 (zmniejsz ul)\n");
 
     // Główna pętla wątku, w której czekamy na sygnały
     while (!stop)
     {
         pause(); // Zawieszenie do czasu nadejścia sygnału
     }
 
     printf("[Beekeeper] Kończę działanie wątku.\n");
     return NULL;
 }
 
 /**
  * Obsługa sygnału SIGUSR1
  *  - Powiększa ul do min(2*N, capacity * 2)
  */
 static void handle_sigusr1(int signo)
 {
     (void)signo;  // Nie używamy bezpośrednio numeru sygnału
     if (stop) return;
 
     // Odczyt bieżącej pojemności ula w sekcji krytycznej
     lock_queue();
     int current = eggQueue->capacity;
     unlock_queue();
 
     // Proponowana nowa pojemność = 2 * current
     int new_cap = current * 2;
     // Ale nie przekraczamy 2 * N
     if (new_cap > 2 * N)
     {
         new_cap = 2 * N;
     }
 
     // Wywołanie funkcji dostosowującej semafory i capacity
     adjust_hive_capacity(new_cap);
 }
 
 /**
  * Obsługa sygnału SIGUSR2
  *  - Zmniejsza ul do max(1, capacity / 2)
  */
 static void handle_sigusr2(int signo)
 {
     (void)signo;
     if (stop) return;
 
     // Odczyt bieżącej pojemności ula w sekcji krytycznej
     lock_queue();
     int current = eggQueue->capacity;
     unlock_queue();
 
     // Proponowana nowa pojemność = current / 2
     int new_cap = current / 2;
     // Ale nie może spaść poniżej 1
     if (new_cap < 1)
     {
         new_cap = 1;
     }
 
     // Wywołanie funkcji dostosowującej semafory i capacity
     adjust_hive_capacity(new_cap);
 }
 