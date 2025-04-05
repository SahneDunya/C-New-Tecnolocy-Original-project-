#ifndef CNT_HTTP_H
#define CNT_HTTP_H

#include <stdbool.h>
#include <stddef.h>
#include "cnt_socket.h"

// HTTP İstemci Fonksiyonu
char* http_get(const char* url, size_t* response_size);

// HTTP Sunucu Fonksiyonu
bool http_server(const char* address, int port, void (*request_handler)(Socket* client_socket, const char* request));

#endif