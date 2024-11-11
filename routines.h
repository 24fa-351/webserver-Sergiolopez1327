#ifndef ROUTINES_H
#define ROUTINES_H
#include "http_message.h"

void handle_static(int client_socket, const char* path);

void handle_stats(int client_socket);

void handle_calc(int client_socket, const char* query);

#endif
