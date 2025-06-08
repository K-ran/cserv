#include "config.h"
#include "cserve.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// globals
static int port = DEFAULT_PORT;
static char directory[MAX_DIR_PATH_SIZE] = "./";

// Define a structure for command line arguments
typedef struct {
    const char *short_flag;
    const char *long_flag;
    const char *arg_name;
    const char *description;
} arg_t;

// Create an array of these structures
static const arg_t valid_args[] = {
    {"-p", "--port", "port", "Port number to listen on"},
    {"-h", "--help", "help", "Display this help message"},
    {"-d", "--directory", "directory", "Root directory to serve"},
    {"-v", "--version", "version", "Display the version of the server"},
};

/**
 * @brief Check if the argument is valid
 *
 * @param arg Argument to check
 * @return SUCCESS if valid, FAILURE if invalid
 */
int check_arg_validity(char *arg) {
    int num_args = sizeof(valid_args) / sizeof(valid_args[0]);
    for (int i = 0; i < num_args; i++) {
        if (strcmp(arg, valid_args[i].short_flag) == 0 ||
            strcmp(arg, valid_args[i].long_flag) == 0) {
            return SUCCESS;
        }
    }
    return FAILURE;
}

/**
 * @brief Print the help message
 */
void print_help() {
    printf("Usage: cserv [options]\n");
    printf("Options:\n");
    int num_args = sizeof(valid_args) / sizeof(valid_args[0]);
    for (int i = 0; i < num_args; i++) {
        printf("  %s, %s\t%s\n", valid_args[i].short_flag, valid_args[i].long_flag,
               valid_args[i].description);
    }
}

int check_if_directory_exists(char *directory) {
    if (access(directory, F_OK) == -1) {
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * @brief Parse the command line arguments
 *
 * @param argc Number of arguments
 * @param argv Array of arguments
 */
int arg_parse(int argc, char *argv[]) {

    // Check if there are any arguments
    if (argc == 1) {
        print_help();
        return FAILURE;
    }

    // Check if any argument is invalid
    for (int i = 1; i < argc; i += 2) {
        if (check_arg_validity(argv[i]) == FAILURE) {
            printf("Error: Invalid argument: %s\n", argv[i]);
            print_help();
            return FAILURE;
        }
    }

    // get port number
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], valid_args[0].short_flag) == 0 ||
            strcmp(argv[i], valid_args[0].long_flag) == 0) {
            port = atoi(argv[i + 1]);
            if (port == 0) {
                printf("Error: Invalid port number: %s\n", argv[i + 1]);
                print_help();
                return FAILURE;
            }
            if (port < 1 || port > 65535) {
                printf("Error:Port number must be between 1 and 65535\n");
                print_help();
                return FAILURE;
            }
            break;
        }
    }

    // get directory
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], valid_args[2].short_flag) == 0 ||
            strcmp(argv[i], valid_args[2].long_flag) == 0) {
            snprintf(directory, MAX_DIR_PATH_SIZE, "%s", argv[i + 1]);

            // convert to absolute path if relative
            if (directory[0] != '/') {
                char cwd[MAX_DIR_PATH_SIZE];
                char temp_path[MAX_DIR_PATH_SIZE * 2];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    snprintf(temp_path, sizeof(temp_path), "%s/%s", cwd, directory);

// Temporarily disable format-truncation warning, I know what I am doing, we check the path later on
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
                    snprintf(directory, MAX_DIR_PATH_SIZE, "%s", temp_path);
#pragma GCC diagnostic pop
                } else {
                    printf("Error: Could not get current working directory\n");
                    print_help();
                    return FAILURE;
                }
                if (check_if_directory_exists(directory) == FAILURE) {
                    printf("Error: Directory does not exist: %s\n", directory);
                    print_help();
                    return FAILURE;
                }
            }
            break;
        }
    }

    return SUCCESS;
}

int main(int argc, char *argv[]) {
    if (arg_parse(argc, argv) == FAILURE) {
        return FAILURE;
    }
    if (cserve_init(port, directory) == FAILURE) {
        return FAILURE;
    }
    if (cserve_start() == FAILURE) {
        return FAILURE;
    }
    // we should never reach here if everything is working correctly
    return SUCCESS;
}