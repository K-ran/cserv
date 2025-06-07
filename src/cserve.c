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
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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
    // File descriptors for the server socket and client connections
    // In Unix/Linux, everything is a file, including network sockets
    int server_fd, new_socket;

    // Structure to hold socket address information (IP address and port)
    // sockaddr_in is specifically for IPv4 addresses
    struct sockaddr_in address;

    // Option value for socket configuration (1 = enable, 0 = disable)
    int opt = 1;

    // Size of the address structure - needed for accept() function
    int addrlen = sizeof(address);

    // Buffer to store incoming HTTP requests from clients
    // We don't parse the request in this simple server, but we need to read it
    char buffer[1024] = {0};

    // Complete HTTP response that we'll send to every client
    // HTTP/1.1 200 OK = HTTP version 1.1, status code 200 (success)
    // Content-Type: text/html = tells browser this is HTML content
    // Connection: close = close connection after sending response
    // \r\n = carriage return + line feed (HTTP line endings)
    // Empty line (\r\n\r\n) separates headers from body
    const char *http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>Karan's Website</title></head>\n"
        "<body>\n"
        "<h1>What's up! This is Karan's website, currently in progress.</h1>\n"
        "</body>\n"
        "</html>\n";

    printf("Starting server on port %d\n", PORT);
    printf("Serving directory %s\n", DIRECTORY);

    // STEP 1: Create a socket
    // socket() creates an endpoint for communication and returns a file descriptor
    // AF_INET = Address Family Internet (IPv4 addresses)
    // SOCK_STREAM = TCP socket (reliable, connection-oriented, ordered data)
    //   Alternative: SOCK_DGRAM for UDP (unreliable, connectionless)
    // 0 = protocol (0 means use default protocol for the socket type, which is TCP for SOCK_STREAM)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return FAILURE;
    }

    // STEP 2: Set socket options
    // setsockopt() configures socket behavior
    // SOL_SOCKET = Socket level (as opposed to protocol-specific levels like IPPROTO_TCP)
    // SO_REUSEADDR = Allow reuse of local addresses
    //   This prevents "Address already in use" error when restarting the server
    //   Without this, you'd have to wait for the OS to clean up the old socket
    // &opt = pointer to option value (1 = enable this option)
    // sizeof(opt) = size of the option value
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return FAILURE;
    }

    // STEP 3: Configure the server address
    // sin_family = address family (must match the socket family)
    address.sin_family = AF_INET;

    // sin_addr.s_addr = IP address to bind to
    // INADDR_ANY = bind to all available network interfaces (0.0.0.0)
    //   This means the server will accept connections from any IP address
    //   Alternative: you could bind to a specific IP like inet_addr("127.0.0.1")
    address.sin_addr.s_addr = INADDR_ANY;

    // sin_port = port number to bind to
    // htons() = Host TO Network Short - converts port number to network byte order
    //   Network byte order is big-endian, but your computer might be little-endian
    //   This ensures the port number is interpreted correctly across different systems
    address.sin_port = htons(PORT);

    // STEP 4: Bind the socket to the address and port
    // bind() assigns the address (IP + port) to the socket
    // This is like claiming "this socket will listen on this specific address and port"
    // (struct sockaddr *) = cast to generic socket address structure
    //   sockaddr_in is IPv4-specific, but bind() expects the generic sockaddr type
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return FAILURE;
    }

    // STEP 5: Start listening for incoming connections
    // listen() marks the socket as passive (ready to accept connections)
    // 3 = backlog size (maximum number of pending connections in the queue)
    //   If more than 3 clients try to connect simultaneously, additional ones will be rejected
    //   This is different from the maximum number of concurrent connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        return FAILURE;
    }

    printf("Server listening on port %d...\n", PORT);
    printf("Visit http://localhost:%d in your browser\n", PORT);

    // STEP 6: Main server loop - handle client connections forever
    while (1) {
        printf("Waiting for connections...\n");

        // accept() waits for and accepts an incoming connection
        // It blocks (waits) until a client connects
        // Returns a NEW socket file descriptor for communicating with this specific client
        // The original server_fd continues listening for more connections
        // (struct sockaddr *)&address = will be filled with client's address info
        // (socklen_t*)&addrlen = size of address structure (input/output parameter)
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) <
            0) {
            perror("accept");
            continue; // If accept fails, try again with the next connection
        }

        // STEP 7: Read the HTTP request from the client
        // read() reads data from the socket into our buffer
        // We don't actually parse the HTTP request in this simple server
        // But we need to read it to clear the socket buffer
        // 1024 = maximum number of bytes to read
        read(new_socket, buffer, 1024);
        printf("Request received\n");

        // STEP 8: Send our HTTP response back to the client
        // send() writes data to the socket
        // strlen(http_response) = number of bytes to send
        // 0 = flags (no special options)
        send(new_socket, http_response, strlen(http_response), 0);
        printf("Response sent\n");

        // STEP 9: Close the connection with this client
        // close() closes the socket file descriptor
        // This ends the connection with this specific client
        // The server continues running and can accept new connections
        close(new_socket);
    }

    // This code will never be reached because we have an infinite loop above
    // But it's good practice to clean up resources
    close(server_fd);
    return SUCCESS;
}