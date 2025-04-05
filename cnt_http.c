#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_http.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#endif

// HTTP İstemci Fonksiyonu
char* http_get(const char* url, size_t* response_size) {
    // URL'yi ayrıştır
    char hostname[256];
    char path[256];
    sscanf(url, "http://%[^/]%s", hostname, path);

    // Soket oluştur
    Socket* socket = create_socket(AF_INET, SOCK_STREAM, 0);
    if (socket == NULL) {
        return NULL;
    }

    // Sunucuya bağlan
    if (!connect_socket(socket, hostname, 80)) {
        close_socket(socket);
        free_socket(socket);
        return NULL;
    }

    // HTTP GET isteği oluştur
    char request[1024];
    sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);

    // İsteği gönder
    send_socket(socket, request, strlen(request));

    // Yanıtı al
    char response[4096];
    int bytes_received = receive_socket(socket, response, sizeof(response) - 1);
    if (bytes_received <= 0) {
        close_socket(socket);
        free_socket(socket);
        return NULL;
    }
    response[bytes_received] = '\0';

    // Soketi kapat
    close_socket(socket);
    free_socket(socket);

    // Yanıtı döndür
    *response_size = bytes_received;
    return strdup(response);
}

// HTTP Sunucu Fonksiyonu
bool http_server(const char* address, int port, void (*request_handler)(Socket* client_socket, const char* request)) {
    // Soket oluştur
    Socket* server_socket = create_socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == NULL) {
        return false;
    }

    // Soketi bağla
    if (!bind_socket(server_socket, address, port)) {
        close_socket(server_socket);
        free_socket(server_socket);
        return false;
    }

    // Soketi dinle
    if (!listen_socket(server_socket, 10)) {
        close_socket(server_socket);
        free_socket(server_socket);
        return false;
    }

    // İstekleri kabul et
    while (true) {
        Socket* client_socket = accept_socket(server_socket);
        if (client_socket == NULL) {
            continue;
        }

        // İsteği al
        char request[4096];
        int bytes_received = receive_socket(client_socket, request, sizeof(request) - 1);
        if (bytes_received <= 0) {
            close_socket(client_socket);
            free_socket(client_socket);
            continue;
        }
        request[bytes_received] = '\0';

        // İsteği işle
        request_handler(client_socket, request);

        // Soketi kapat
        close_socket(client_socket);
        free_socket(client_socket);
    }

    // Sunucu soketini kapat
    close_socket(server_socket);
    free_socket(server_socket);
    return true;
}