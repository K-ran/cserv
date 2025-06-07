/**
 * @file cserve.c
 * @brief Implementation of the cserve library
 *
 * @author Karan Purohit
 * @date 10/10/25
 */
#include "cserve.h"
#include "config.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

// globals
int PORT;
char DIRECTORY[MAX_DIR_PATH_SIZE];

/**
 * @brief Initialize the server
 *
 * @param port Port number to listen on
 * @param directory Root directory to serve
 * @return SUCCESS on success, negative value on error
 */
int cserve_init(int port, const char *directory) {
    // Initialize the server
    PORT = port;
    strncpy(DIRECTORY, directory, MAX_DIR_PATH_SIZE);
    return SUCCESS;
}

/**
 * @brief Start the server
 *
 * @return SUCCESS on success, negative value on error
 */
int cserve_start() {
    // Start the server
    printf("Starting server on port %d\n", PORT);
    printf("Serving directory %s\n", DIRECTORY);

    return SUCCESS;
}