#include "echo.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "http_message.h"
#include "routines.h"

int request_count = 0;
size_t total_received_bytes = 0;
size_t total_sent_bytes = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_connection(void* arg) {
    int client_socket = *(int*) arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Client connected.\n");

    http_client_message_t* http_message;
    http_read_result_t result;
    read_http_client_message(client_socket, &http_message, &result);
    if (result == BAD_REQUEST) {
        printf("Bad request.\n");
        const char* bad_request_response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n";
        send(client_socket, bad_request_response, strlen(bad_request_response),
             0);
        close(client_socket);
        return NULL;
    } else if (result == CLOSED_CONNECTION) {
        printf("Closed connection.\n");
        close(client_socket);
        return NULL;
    }

    pthread_mutex_lock(&stats_mutex);
    request_count++;
    pthread_mutex_unlock(&stats_mutex);

    const char* path = http_message->path;
    if (strncmp(path, "/static/", 8) == 0) {
        handle_static(client_socket, path);
    } else if (strcmp(path, "/stats") == 0) {
        handle_stats(client_socket);
    } else if (strncmp(path, "/calc/", 6) == 0) {
        handle_calc(client_socket, path + 6);
    } else {
        const char* not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n";
        if (send(client_socket, not_found_response, strlen(not_found_response),
                 0) == -1) {
            perror("send error for 404 response");
        }
    }

    http_client_message_free(http_message);
    close(client_socket);
    return NULL;
}
