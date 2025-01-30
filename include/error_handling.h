#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

typedef enum {
    ERR_SUCCESS = 0,
    ERR_INVALID_INPUT,
    ERR_THREAD_FAILURE,
    ERR_PROCESS_FAILURE,
    ERR_MEMORY_ALLOC,
    ERR_SEMAPHORE_INIT,
    ERR_UNKNOWN
} ErrorCode;

typedef struct {
    ErrorCode code;
    const char *message;
} Error;

// Prosta funkcja wypisująca błąd na stderr
void handle_error(Error err);

#endif // ERROR_HANDLING_H
