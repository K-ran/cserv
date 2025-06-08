/**
 * @file cserve_net.c
 * @brief Network-related functions for HTTP request parsing
 *
 * @author Karan Purohit
 * @date 10/10/25
 */

// Define feature macros before including headers
// These enable POSIX functions like strncasecmp and strdup
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "cserve_net.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  // For strncasecmp

/**
 * @brief Helper function to trim whitespace from both ends of a string
 *
 * Removes leading and trailing spaces, tabs, carriage returns, and newlines.
 * This is needed because HTTP headers often have extra whitespace.
 *
 * @param str The string to trim (modified in place)
 */
static void trim_whitespace(char *str) {
    if (str == NULL)
        return;

    // Find the first non-whitespace character
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // If string is all whitespace, make it empty
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }

    // Find the last non-whitespace character
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Calculate length and move string to beginning
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';
}

/**
 * @brief Extract value from an HTTP header line
 *
 * HTTP headers have format "Header-Name: Header-Value"
 * This function extracts the value part after the colon.
 *
 * @param line The header line to parse
 * @param header_name The name of the header to look for
 * @param value Buffer to store the extracted value
 * @param value_size Size of the value buffer
 * @return 1 if header found and extracted, 0 otherwise
 */
static int extract_header_value(const char *line, const char *header_name, char *value,
                                size_t value_size) {
    // Check if this line starts with the header name (case-insensitive)
    size_t header_len = strlen(header_name);
    if (strncasecmp(line, header_name, header_len) != 0) {
        return 0; // Not the header we're looking for
    }

    // Look for the colon separator
    const char *colon = strchr(line, ':');
    if (colon == NULL) {
        return 0; // Malformed header line
    }

    // Extract the value part (everything after the colon)
    const char *val_start = colon + 1;
    strncpy(value, val_start, value_size - 1);
    value[value_size - 1] = '\0'; // Ensure null termination

    // Remove leading/trailing whitespace from the value
    trim_whitespace(value);
    return 1;
}

/**
 * @brief Parse an HTTP request from raw text
 *
 * This function takes the complete HTTP request as received from the client
 * and extracts the important information into a structured format.
 *
 * Example input:
 * "GET /index.html HTTP/1.1\r\n
 *  Host: localhost\r\n
 *  User-Agent: Mozilla/5.0...\r\n
 *  \r\n"
 *
 * @param raw_request The complete HTTP request text
 * @return Pointer to parsed request structure, or NULL if parsing failed
 */
cserver_http_req_t *parse_http_request(const char *raw_request) {
    if (raw_request == NULL) {
        printf("Error: Raw request is NULL\n");
        return NULL;
    }

    // Allocate memory for the parsed request structure
    cserver_http_req_t *req = malloc(sizeof(cserver_http_req_t));
    if (req == NULL) {
        printf("Error: Memory allocation failed\n");
        return NULL; // Memory allocation failed
    }

    // Initialize all fields to empty strings
    memset(req, 0, sizeof(cserver_http_req_t));

    // Create a working copy of the request since we'll modify it
    char *request_copy = strdup(raw_request);
    if (request_copy == NULL) {
        free(req);
        printf("Error: Memory allocation failed\n");
        return NULL; // Memory allocation failed
    }

    // Split the request into lines using thread-safe strtok_r
    // strtok_r maintains state in saveptr instead of static variables
    char *saveptr1;  // State pointer for line tokenization
    char *line = strtok_r(request_copy, "\r\n", &saveptr1);
    if (line == NULL) {
        free(req);
        free(request_copy);
        printf("Error: Empty request\n");
        return NULL; // Empty request
    }

    // STEP 1: Parse the request line (first line)
    // Format: "METHOD /path HTTP/version"
    // Example: "GET /index.html HTTP/1.1"
    char *saveptr2;  // State pointer for request line tokenization
    char *method = strtok_r(line, " ", &saveptr2);
    char *path = strtok_r(NULL, " ", &saveptr2);
    char *version = strtok_r(NULL, " ", &saveptr2);

    if (method == NULL || path == NULL || version == NULL) {
        free(req);
        free(request_copy);
        return NULL; // Malformed request line
    }

    // Copy the parsed values to our structure
    strncpy(req->method, method, sizeof(req->method) - 1);
    strncpy(req->path, path, sizeof(req->path) - 1);
    strncpy(req->version, version, sizeof(req->version) - 1);

    // STEP 2: Parse the header lines
    // Continue reading lines until we hit an empty line or end of request
    while ((line = strtok_r(NULL, "\r\n", &saveptr1)) != NULL && strlen(line) > 0) {
        // Try to extract each header we're interested in

        if (extract_header_value(line, "Host", req->host, sizeof(req->host))) {
            // Host header found and extracted
            continue;
        }

        if (extract_header_value(line, "User-Agent", req->user_agent, sizeof(req->user_agent))) {
            // User-Agent header found and extracted
            continue;
        }

        if (extract_header_value(line, "Accept", req->accept, sizeof(req->accept))) {
            // Accept header found and extracted
            continue;
        }

        if (extract_header_value(line, "Connection", req->connection, sizeof(req->connection))) {
            // Connection header found and extracted
            continue;
        }

        // If we reach here, it's a header we don't care about, so we ignore it
    }

    // Clean up the working copy
    free(request_copy);

    // Basic validation: we must have at least method and path
    if (strlen(req->method) == 0 || strlen(req->path) == 0) {
        free(req);
        return NULL; // Invalid request
    }

    return req; // Success!
}


/**
 * @brief Print an HTTP request for debugging
 *
 * @param req The request to print
 */
void print_http_request(cserver_http_req_t *req) {
    if (req == NULL) {
        printf("Request is NULL\n");
        return;
    }
    printf("\n----------------------------------------\n");
    printf("HTTP Request:\n");
    printf("Method: %s\n", req->method);
    printf("Path: %s\n", req->path);
    printf("Version: %s\n", req->version);
    printf("Host: %s\n", req->host);
    if (strlen(req->user_agent) > 0) {
        printf("User-Agent: %s\n", req->user_agent);
    }
    if (strlen(req->accept) > 0) {
        printf("Accept: %s\n", req->accept);
    }
    if (strlen(req->connection) > 0) {
        printf("Connection: %s\n", req->connection);
    }
    printf("----------------------------------------\n");

}