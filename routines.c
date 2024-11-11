#include "routines.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "echo.h"

extern int request_count;
extern size_t total_received_bytes;
extern size_t total_sent_bytes;
extern pthread_mutex_t stats_mutex;

void handle_static(int client_socket, const char* path) {
    char file_path[BUFFER_SIZE];
    snprintf(file_path, sizeof(file_path), "%s%s", STATIC_DIR, path + 7);
    FILE* file = fopen(file_path, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        char* file_buffer = malloc(file_size);
        fread(file_buffer, 1, file_size, file);
        fclose(file);

        char response_header[BUFFER_SIZE];
        snprintf(response_header, sizeof(response_header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Length: %zu\r\n"
                 "Content-Type: application/octet-stream\r\n"
                 "\r\n",
                 file_size);
        send(client_socket, response_header, strlen(response_header), 0);
        send(client_socket, file_buffer, file_size, 0);

        pthread_mutex_lock(&stats_mutex);
        total_sent_bytes += strlen(response_header) + file_size;
        pthread_mutex_unlock(&stats_mutex);

        free(file_buffer);
    } else {
        const char* not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
    }
}

void handle_stats(int client_socket) {
    char response_body[BUFFER_SIZE];
    pthread_mutex_lock(&stats_mutex);
    snprintf(response_body, sizeof(response_body),
             "<html><body>"
             "<p>Requests: %d</p>"
             "<p>Received bytes: %zu</p>"
             "<p>Sent bytes: %zu</p>"
             "<p>a + b = sum</p>"
             "</body></html>",
             request_count, total_received_bytes, total_sent_bytes);
    pthread_mutex_unlock(&stats_mutex);

    char response_header[BUFFER_SIZE];
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %zu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n",
             strlen(response_body));
    if (send(client_socket, response_header, strlen(response_header), 0) ==
        -1) {
        perror("send error");
        return;
    }
    if (send(client_socket, response_body, strlen(response_body), 0) == -1) {
        perror("send error");
        return;
    }

    pthread_mutex_lock(&stats_mutex);
    total_sent_bytes += strlen(response_header) + strlen(response_body);
    pthread_mutex_unlock(&stats_mutex);
}

void handle_calc(int client_socket, const char* query) {
    int a = 0, b = 0;
    char* param = strstr(query, "a=");
    if (param) {
        a = atoi(param + 2);
    }
    param = strstr(query, "b=");
    if (param) {
        b = atoi(param + 2);
    }
    int sum = a + b;
    char response_body[BUFFER_SIZE];
    snprintf(response_body, sizeof(response_body),
             "<html><body>Sum = %d + %d = %d</body></html>", a, b, sum);

    char response_header[BUFFER_SIZE];
    snprintf(response_header, sizeof(response_header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Length: %zu\r\n"
             "Content-Type: text/html\r\n"
             "\r\n",
             strlen(response_body));
    if (send(client_socket, response_header, strlen(response_header), 0) ==
        -1) {
        perror("send error");
        return;
    }
    if (send(client_socket, response_body, strlen(response_body), 0) == -1) {
        perror("send error");
        return;
    }

    pthread_mutex_lock(&stats_mutex);
    total_sent_bytes += strlen(response_header) + strlen(response_body);
    pthread_mutex_unlock(&stats_mutex);
}