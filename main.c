#include <arpa/inet.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "echo.h"

#ifndef LISTEN_BACKLOG
#define LISTEN_BACKLOG 10  // Define a default backlog if not set
#endif

void cleanup_socket(int server_socket) {
    close(server_socket);
    printf("Server socket closed.\n");
}

int main(int argc, char* argv[]) {
    int port = 80;
    int option;

    while ((option = getopt(argc, argv, "p:")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Invalid port number: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*) &server_address,
             sizeof(server_address)) == -1) {
        perror("bind");
        cleanup_socket(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, LISTEN_BACKLOG) == -1) {
        perror("listen");
        cleanup_socket(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        int* client_socket = malloc(sizeof(int));
        if (!client_socket) {
            perror("malloc");
            continue;
        }

        *client_socket =
            accept(server_socket, (struct sockaddr*) &client_address,
                   &client_address_length);
        if (*client_socket == -1) {
            perror("accept");
            free(client_socket);
            continue;
        }

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_connection,
                           client_socket) != 0) {
            perror("pthread_create");
            close(*client_socket);
            free(client_socket);
            continue;
        }

        pthread_detach(client_thread);  // Detach to prevent resource leaks
    }

    cleanup_socket(server_socket);
    return 0;
}
