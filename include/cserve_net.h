#ifndef CSERVE_NET_H
#define CSERVE_NET_H

/**
 * cserve_net.h
 *
 * Created by Karan Purohit on 10/10/25.
 *
 * Network related functions
 */

/**
 * @brief Structure to hold HTTP request information
 *
 * This structure contains the essential parts of an HTTP request that we need
 * for basic web server functionality. Not all HTTP headers are included,
 * only the most commonly used ones.
 */
typedef struct {
    // HTTP method (GET, POST, PUT, DELETE, etc.)
    // Most web requests are GET requests for retrieving web pages
    char method[16];

    // Requested URL path (e.g., "/", "/index.html", "/about")
    // This tells us what resource the client wants
    char path[512];

    // HTTP version (e.g., "HTTP/1.1", "HTTP/1.0")
    // Different versions have different capabilities
    char version[16];

    // Host header - which domain/server the client thinks it's talking to
    // Important for virtual hosting (multiple websites on one server)
    char host[256];

    // User-Agent header - identifies the client software (browser, etc.)
    // Useful for logging and sometimes for serving different content
    char user_agent[512];

    // Accept header - what content types the client can handle
    // Helps server decide what format to send (HTML, JSON, etc.)
    char accept[256];

    // Connection header - whether to keep connection alive or close it
    // "keep-alive" means reuse connection, "close" means close after response
    char connection[32];

} cserver_http_req_t;

/**
 * @brief Parse an HTTP request from raw text
 *
 * Takes the raw HTTP request text received from a client socket and
 * extracts the important information into a structured format.
 *
 * @param raw_request The raw HTTP request text from the client
 * @return Pointer to parsed request structure, or NULL if parsing failed
 *
 * Note: The returned pointer must be freed by the caller using free()
 */
cserver_http_req_t *parse_http_request(const char *raw_request);

/**
 * @brief Print an HTTP request for debugging
 *
 * @param req The request to print
 */
void print_http_request(cserver_http_req_t *req);

#endif