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
#include <strings.h> // For strncasecmp
#include <time.h>    // For time functions

/*
 * Function to convert method string to enum
 *
 */
cserver_http_method_t method_str_to_enum(const char *method) {
    if (strcasecmp(method, "GET") == 0) {
        return HTTP_METHOD_GET;
    } else if (strcasecmp(method, "HEAD") == 0) {
        return HTTP_METHOD_HEAD;
    } else if (strcasecmp(method, "POST") == 0) {
        return HTTP_METHOD_POST;
    } else if (strcasecmp(method, "PUT") == 0) {
        return HTTP_METHOD_PUT;
    } else if (strcasecmp(method, "DELETE") == 0) {
        return HTTP_METHOD_DELETE;
    } else if (strcasecmp(method, "CONNECT") == 0) {
        return HTTP_METHOD_CONNECT;
    } else if (strcasecmp(method, "OPTIONS") == 0) {
        return HTTP_METHOD_OPTIONS;
    } else if (strcasecmp(method, "TRACE") == 0) {
        return HTTP_METHOD_TRACE;
    } else if (strcasecmp(method, "PATCH") == 0) {
        return HTTP_METHOD_PATCH;
    } else {
        return -1;
    }
}
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
    char *saveptr1; // State pointer for line tokenization
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
    char *saveptr2; // State pointer for request line tokenization
    char *method = strtok_r(line, " ", &saveptr2);
    char *path = strtok_r(NULL, " ", &saveptr2);
    char *version = strtok_r(NULL, " ", &saveptr2);

    if (method == NULL || path == NULL || version == NULL) {
        free(req);
        free(request_copy);
        printf("Error: Malformed request line - missing method, path, or version\n");
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
        printf("Error: Invalid request - empty method or path\n");
        return NULL; // Invalid request
    }

    return req; // Success!
}

/**
 * @brief Get the status message for a given HTTP status code
 *
 * Helper function that returns the standard HTTP status message
 * for common status codes.
 *
 * @param status_code The HTTP status code
 * @return Pointer to the status message string
 */
static const char *get_status_message(cserver_http_status_t status_code) {
    switch (status_code) {
    case HTTP_STATUS_OK:
        return "OK";
    case HTTP_STATUS_CREATED:
        return "Created";
    case HTTP_STATUS_NO_CONTENT:
        return "No Content";
    case HTTP_STATUS_BAD_REQUEST:
        return "Bad Request";
    case HTTP_STATUS_UNAUTHORIZED:
        return "Unauthorized";
    case HTTP_STATUS_FORBIDDEN:
        return "Forbidden";
    case HTTP_STATUS_NOT_FOUND:
        return "Not Found";
    case HTTP_STATUS_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case HTTP_STATUS_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    case HTTP_STATUS_NOT_IMPLEMENTED:
        return "Not Implemented";
    case HTTP_STATUS_SERVICE_UNAVAILABLE:
        return "Service Unavailable";
    default:
        return "Unknown";
    }
}

/**
 * @brief Create a default HTTP response with common headers populated
 *
 * This function creates a complete HTTP response structure with all
 * the essential headers filled in with sensible defaults.
 */
cserver_http_res_t *create_http_response(cserver_http_status_t status_code,
                                         const char *content_type, const char *body) {
    if (content_type == NULL) {
        printf("Error: Content type is required for HTTP response\n");
        return NULL; // Content type is required
    }

    // Allocate memory for the response structure
    cserver_http_res_t *response = malloc(sizeof(cserver_http_res_t));
    if (response == NULL) {
        printf("Error: Memory allocation failed for HTTP response structure\n");
        return NULL; // Memory allocation failed
    }

    // Initialize all fields to zero
    memset(response, 0, sizeof(cserver_http_res_t));

    // Set HTTP version (we always use HTTP/1.1)
    strncpy(response->version, "HTTP/1.1", sizeof(response->version) - 1);

    // Set status code and message
    response->status_code = status_code;
    strncpy(response->status_message, get_status_message(status_code),
            sizeof(response->status_message) - 1);

    // Generate current date in HTTP format
    // HTTP date format: "Sun, 06 Nov 1994 08:49:37 GMT"
    time_t now = time(NULL);
    struct tm *gmt = gmtime(&now);
    strftime(response->date, sizeof(response->date), "%a, %d %b %Y %H:%M:%S GMT", gmt);

    // Set server identification
    strncpy(response->server, "CServer/1.0", sizeof(response->server) - 1);

    // Set content type
    strncpy(response->content_type, content_type, sizeof(response->content_type) - 1);

    // Set connection to close (simple approach for now)
    strncpy(response->connection, "close", sizeof(response->connection) - 1);

    // Handle the body content
    if (body != NULL) {
        // Calculate content length
        response->content_length = strlen(body);

        // Allocate and copy the body
        response->body = malloc(response->content_length + 1);
        if (response->body == NULL) {
            free(response);
            printf("Error: Memory allocation failed for HTTP response body\n");
            return NULL; // Memory allocation failed
        }
        strcpy(response->body, body);
    } else {
        // No body content
        response->content_length = 0;
        response->body = NULL;
    }

    return response;
}

/**
 * @brief Convert HTTP response structure to byte stream for transmission
 *
 * This function formats the HTTP response into the exact string format
 * required by the HTTP protocol for transmission over a socket.
 */
char *http_response_to_string(const cserver_http_res_t *response) {
    if (response == NULL) {
        printf("Error: HTTP response structure is NULL\n");
        return NULL;
    }

    // Calculate the size needed for the complete response
    // Headers typically need about 300-500 bytes, plus body length
    size_t header_size = 512;
    size_t total_size = header_size + response->content_length + 1;

    // Allocate memory for the complete response string
    char *response_str = malloc(total_size);
    if (response_str == NULL) {
        printf("Error: Memory allocation failed for HTTP response string\n");
        return NULL; // Memory allocation failed
    }

    // Format the HTTP response according to protocol
    // Status line: HTTP/1.1 200 OK\r\n
    int written = snprintf(response_str, total_size,
                           "%s %d %s\r\n"
                           "Date: %s\r\n"
                           "Server: %s\r\n"
                           "Content-Type: %s\r\n"
                           "Content-Length: %zu\r\n"
                           "Connection: %s\r\n"
                           "\r\n", // Empty line separates headers from body
                           response->version, response->status_code, response->status_message,
                           response->date, response->server, response->content_type,
                           response->content_length, response->connection);

    // Check if header formatting was successful
    if (written < 0 || (size_t)written >= header_size) {
        free(response_str);
        printf("Error: HTTP response header formatting failed or buffer too small\n");
        return NULL; // Formatting error or buffer too small
    }


    // Append the body content if it exists
    if (response->body != NULL && response->content_length > 0) {
        // Make sure we have enough space
        if ((size_t)written + response->content_length >= total_size) {
            free(response_str);
            printf("Error: Not enough buffer space for HTTP response body\n");
            return NULL; // Not enough space
        }

        printf("Response created\n");
        printf("Debug: total size: %lu\n", total_size);
        printf("Debug: written: %d\n", written);
        printf("Debug: content length: %ld\n", response->content_length);

        // Append the body to the response
        memcpy(response_str + written, response->body, response->content_length);
        response_str[written + response->content_length] = '\0';
    }

    return response_str;
}

/**
 * @brief Free memory allocated for HTTP response structure
 *
 * This function properly cleans up all memory associated with
 * an HTTP response structure.
 */
void free_http_response(cserver_http_res_t *response) {
    if (response == NULL) {
        return; // Nothing to free
    }

    // Free the body content if it was allocated
    if (response->body != NULL) {
        free(response->body);
        response->body = NULL;
    }

    // Free the response structure itself
    free(response);
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