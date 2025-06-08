#ifndef CSERVE_NET_H
#define CSERVE_NET_H

/**
 * cserve_net.h
 *
 * Created by Karan Purohit on 10/10/25.
 *
 * Network related functions
 */

#include <stddef.h>

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

/**
 * @brief HTTP status codes enum
 *
 * Contains the most commonly used HTTP status codes for web server responses.
 * Each status code has a specific meaning defined by the HTTP protocol.
 */
typedef enum {
    HTTP_STATUS_OK = 200,                    // Request successful
    HTTP_STATUS_CREATED = 201,               // Resource created successfully
    HTTP_STATUS_NO_CONTENT = 204,            // Success but no content to return
    HTTP_STATUS_BAD_REQUEST = 400,           // Client sent invalid request
    HTTP_STATUS_UNAUTHORIZED = 401,          // Authentication required
    HTTP_STATUS_FORBIDDEN = 403,             // Server understood but refuses to authorize
    HTTP_STATUS_NOT_FOUND = 404,             // Requested resource not found
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,    // HTTP method not supported for resource
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500, // Server encountered unexpected condition
    HTTP_STATUS_NOT_IMPLEMENTED = 501,       // Server doesn't support functionality
    HTTP_STATUS_SERVICE_UNAVAILABLE = 503    // Server temporarily overloaded or down
} cserver_http_status_t;

/**
 * @brief HTTP Methods enum
 *
 * Contains the most commonly used HTTP methods for web server requests.
 * TODO: Not everything here is supported.
 */
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH
} cserver_http_method_t;

/*
 * Function to convert method string to enum
 *
 */
cserver_http_method_t method_str_to_enum(const char *method);

/**
 * @brief Structure to hold HTTP response information
 *
 * This structure contains all the essential parts of an HTTP response
 * that a basic web server needs to send back to clients.
 */
typedef struct {
    // HTTP version (e.g., "HTTP/1.1")
    char version[16];

    // HTTP status code (200, 404, 500, etc.)
    int status_code;

    // Status message (e.g., "OK", "Not Found", "Internal Server Error")
    char status_message[64];

    // Date header - when the response was generated
    char date[64];

    // Server header - identifies our server software
    char server[64];

    // Content-Type header - what type of content we're sending
    // (e.g., "text/html", "application/json", "image/png")
    char content_type[64];

    // Content-Length header - size of the response body in bytes
    size_t content_length;

    // Connection header - whether to keep connection alive or close it
    char connection[32];

    // Response body - the actual content (HTML, JSON, etc.)
    char *body;

} cserver_http_res_t;

/**
 * @brief Create a default HTTP response with common headers populated
 *
 * Creates a new HTTP response structure with sensible defaults:
 * - HTTP/1.1 protocol version
 * - Current date/time
 * - Server identification
 * - Connection: close
 *
 * @param status_code HTTP status code (e.g., 200, 404, 500)
 * @param content_type MIME type of the content (e.g., "text/html")
 * @param body Response body content (will be copied)
 * @return Pointer to new response structure, or NULL if creation failed
 *
 * Note: The returned pointer must be freed using free_http_response()
 */
cserver_http_res_t *create_http_response(cserver_http_status_t status_code,
                                         const char *content_type, const char *body);

/**
 * @brief Convert HTTP response structure to byte stream for transmission
 *
 * Converts the structured HTTP response into a properly formatted
 * HTTP response string that can be sent over a socket connection.
 *
 * Format:
 * HTTP/1.1 200 OK\r\n
 * Date: ...\r\n
 * Server: ...\r\n
 * Content-Type: ...\r\n
 * Content-Length: ...\r\n
 * Connection: ...\r\n
 * \r\n
 * [body content]
 *
 * @param response Pointer to the HTTP response structure
 * @return Pointer to formatted response string, or NULL if conversion failed
 *
 * Note: The returned string must be freed by the caller using free()
 */
char *http_response_to_string(const cserver_http_res_t *response);

/**
 * @brief Free memory allocated for HTTP response structure
 *
 * Properly deallocates all memory associated with an HTTP response,
 * including the body content and the structure itself.
 *
 * @param response Pointer to the HTTP response structure to free
 */
void free_http_response(cserver_http_res_t *response);

#endif