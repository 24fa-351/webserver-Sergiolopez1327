#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H
#include <stdbool.h>

typedef enum { BAD_REQUEST, CLOSED_CONNECTION, MESSAGE } http_read_result_t;

typedef struct http_client_message {
    char* method;
    char* path;
    char* http_version;
    char* body;
    int body_length;
    char* headers;
} http_client_message_t;

void read_http_client_message(int client_socket,
                              http_client_message_t** http_message,
                              http_read_result_t* result);

int respond_to_http_message(int client_socket,
                            http_client_message_t* http_message);

void http_client_message_free(http_client_message_t* http_message);

bool complete_http_message(char* buffer);

#endif