/**
 * @file cserve.c
 * @brief Implementation of the cserve library
 *
 * @author Karan Purohit
 * @date 10/10/25
 */
#include "cserve.h"
#include "error.h"
#include <stdio.h>

static int init(int port) {
    // Initialize the server
    printf("Initializing server on port %d\n", port);
    return 0;
}