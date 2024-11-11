#include "http_message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void read_http_client_message(int client_socket,
                              http_client_message_t** http_message,
                              http_read_result_t* result) {
    *http_message = malloc(sizeof(http_client_message_t));
    if (!*http_message) {
        *result = BAD_REQUEST;
        return;
    }
    memset(*http_message, 0, sizeof(http_client_message_t));

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t total_bytes_read = 0;

    while (!complete_http_message(buffer)) {
        int bytes_read = read(client_socket, buffer + total_bytes_read,
                              sizeof(buffer) - total_bytes_read - 1);
        if (bytes_read == 0) {
            *result = CLOSED_CONNECTION;
            free(*http_message);
            return;
        }
        if (bytes_read < 0) {
            *result = BAD_REQUEST;
            free(*http_message);
            return;
        }
        total_bytes_read += bytes_read;
        buffer[total_bytes_read] = '\0';
    }

    char method[16], path[256], version[16];
    if (sscanf(buffer, "%15s %255s %15s", method, path, version) != 3) {
        *result = BAD_REQUEST;
        free(*http_message);
        return;
    }
    (*http_message)->method = strdup(method);
    (*http_message)->path = strdup(path);
    (*http_message)->http_version = strdup(version);

    *result = MESSAGE;
}

int respond_to_http_message(int client_socket,
                            http_client_message_t* http_message) {
    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";
    write(client_socket, response, strlen(response));
    return 0;
}

void http_client_message_free(http_client_message_t* http_message) {
    if (http_message) {
        free(http_message->method);
        free(http_message->path);
        free(http_message->http_version);
        free(http_message->body);
        free(http_message);
    }
}

bool complete_http_message(char* buffer) {
    return strstr(buffer, "\r\n\r\n") != NULL;
}
