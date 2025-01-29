#include "error_handling.h"
#include <stdio.h>

void handle_error(Error err) {
    if (err.code != ERR_SUCCESS) {
        fprintf(stderr, "Error: %s (Code: %d)\n", err.message, err.code);
    }
}
