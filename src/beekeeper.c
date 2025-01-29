
#include "beekeper.h"

/********************************************
 * Beekeeper thread
 ********************************************/
void* beekeeper(void* arg) {
    // Enable cancellation
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (!stop) {
        printf("Czekam na sygnał (4 = dodaj ramki, 5 = usuń ramki): ");
        fflush(stdout);

        // Setup for select()
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval tv;
        tv.tv_sec = 10;  // 1 second
        tv.tv_usec = 0;

        int ret = select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv);
        if (stop) break; // check after select

        if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            int signal_code;
            if (scanf("%d", &signal_code) == 1) {
                if (stop) break;
                if (signal_code == 4) {
                    // double capacity
                } else if (signal_code == 5) {
                    // halve capacity
                } else {
                    printf("Nieznany sygnał.\n");
                }
            } else {
                // flush the invalid input
                int c;
                while ((c = getchar()) != '\n' && c != EOF) {}
                fprintf(stderr, "Niepoprawne dane. Spróbuj ponownie.\n");
            }
        }
        // If ret == 0 => no input arrived in 1s => loop again
        // If ret < 0 => error occurred, handle if needed
    }

    printf("Beekeeper thread exiting.\n");
    return NULL;
}