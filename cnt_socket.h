#ifndef CNT_SOCKET_H
#define CNT_SOCKET_H

#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

// Soket Yapısı
typedef struct {
    int socket_fd;
} Socket;

// Soket Oluşturma Fonksiyonu
Socket* create_socket(int domain, int type, int protocol);

// Soketi Bağlama Fonksiyonu
bool bind_socket(Socket* socket, const char* address, int port);

// Soketi Dinleme Fonksiyonu
bool listen_socket(Socket* socket, int backlog);

// Bağlantıyı Kabul Etme Fonksiyonu
Socket* accept_socket(Socket* socket);

// Sunucuya Bağlanma Fonksiyonu
bool connect_socket(Socket* socket, const char* address, int port);

// Veri Gönderme Fonksiyonu
int send_socket(Socket* socket, const char* data, size_t size);

// Veri Alma Fonksiyonu
int receive_socket(Socket* socket, char* buffer, size_t size);

// Soketi Kapatma Fonksiyonu
bool close_socket(Socket* socket);

// Soketi Serbest Bırakma Fonksiyonu
void free_socket(Socket* socket);

#endif