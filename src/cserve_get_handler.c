/**
 * @file cserve_get_handler.c
 * @brief Implementation of the GET request handler
 *
 * @author Karan Purohit
 * @date 10/10/25
 */

#include "cserve_get_handler.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "error.h"


/**
 * @brief Validate the path
 *
 * @param path The path to validate
 * @return SUCCESS on success, FAILURE on failure
 */
int validate_path(const char *path) {
    // Check for null path
    if (path == NULL) {
        return FAILURE;
    }
    // Check for absolute path
    if (path[0] != '/') {
        return FAILURE;
    }
    // Make sure only valid characters are used
    // Valid characters are: a-z, A-Z, 0-9, /, _, ., -
    // Fail if any other characters are used`
    // Also .. is not allowed
    if (strstr(path, "..") != NULL) {
        return FAILURE;
    }
    for (size_t i = 0; i < strlen(path); i++) {
        if (path[i] == '/' ||
            path[i] == '_' ||
            path[i] == '.' ||
            path[i] == '-' ||
            (path[i] >= 'a' && path[i] <= 'z') ||
            (path[i] >= 'A' && path[i] <= 'Z') ||
            (path[i] >= '0' && path[i] <= '9')) {
            continue;
        } else {
            return FAILURE;
        }
    }
    return SUCCESS;
}

/**
 * @brief Handle a GET request
 *
 * @param req The request to handle
 * @param root_dir The root directory to serve
 * @return cserver_http_res_t* The response
*/
cserver_http_res_t *cserve_get_handler(cserver_http_req_t *req, const char *root_dir) {
    // Check if the request is a GET request
    if (strcmp(req->method, "GET") != 0) {
        return create_http_response(HTTP_STATUS_METHOD_NOT_ALLOWED, "text/plain", "Method Not Allowed");
    }

    // set the content type based on the file extension
    char *content_type = "text/plain";

    // Check if the path is valid and there is no funny business going on
    if (validate_path(req->path) == FAILURE) {
        printf("Error: Invalid path: %s\n", req->path);
        return create_http_response(HTTP_STATUS_BAD_REQUEST, "text/plain", "Bad Request");
    }

    // if path is "/", serve index.html
    if (strcmp(req->path, "/") == 0) {
        content_type = "text/html";
        strncpy(req->path, "/index.html", sizeof(req->path) - 1);
    } 
    if (strstr(req->path, ".html") != NULL) {
        content_type = "text/html";
    } else if (strstr(req->path, ".css") != NULL) {
        content_type = "text/css";
    } else if (strstr(req->path, ".js") != NULL) {
        content_type = "application/javascript";
    } else if (strstr(req->path, ".png") != NULL) {
        content_type = "image/png";
    } else if (strstr(req->path, ".jpg") != NULL) {
        content_type = "image/jpeg";
    } else if (strstr(req->path, ".jpeg") != NULL) {
        content_type = "image/jpeg";
    } 
    else if (strstr(req->path, ".ico") != NULL) {
        content_type = "image/x-icon";
    }

    printf("Path: %s\n", req->path);
    printf("Content type: %s\n", content_type);

    // Check if the requested file exists
    char file_path[MAX_DIR_PATH_SIZE];
    snprintf(file_path, sizeof(file_path), "%s%s", root_dir, req->path);
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Error: File not found: %s\n", file_path);
        return create_http_response(HTTP_STATUS_NOT_FOUND, content_type, "Not Found");
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *file_content = malloc(file_size + 1);
    if (file_content == NULL) {
        fclose(file);
        printf("Error: Memory allocation failed\n");
        return create_http_response(HTTP_STATUS_INTERNAL_SERVER_ERROR, content_type, "Internal Server Error");
    }

    // Read the file content
    size_t bytes_read = fread(file_content, file_size, 1, file);
    if (bytes_read != 1) {
        free(file_content);
        fclose(file);
        printf("Error: Failed to read file (read %zu of %ld bytes)\n", bytes_read, file_size);
        return create_http_response(HTTP_STATUS_INTERNAL_SERVER_ERROR, content_type, "Internal Server Error");
    }
    file_content[file_size] = '\0';

    // Close the file
    fclose(file);

    // Create the HTTP response
    cserver_http_res_t *response = create_http_response(HTTP_STATUS_OK, content_type, file_content);
    if (response == NULL) {
        free(file_content);
        printf("Error: Failed to create HTTP response\n");
        return NULL;
    }

    // Set the content length
    response->content_length = file_size;

    // Dont free the file content, it's now owned by the response
    // Freeing it will be handled by free_http_response()

    return response;
}