#ifndef ECHO_H
#define ECHO_H

#define BUFFER_SIZE 4096
#define LISTEN_BACKLOG 10
#define STATIC_DIR "static"

void* handle_connection(void* arg);

#endif
