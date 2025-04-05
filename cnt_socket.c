#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnt_socket.h"

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Platforma özgü kodlar (örnek olarak Windows ve Linux)

#ifdef _WIN32

Socket* create_socket(int domain, int type, int protocol) {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return NULL;
    }

    Socket* socket = (Socket*)malloc(sizeof(Socket));
    socket->socket_fd = ::socket(domain, type, protocol);
    if (socket->socket_fd == INVALID_SOCKET) {
        free(socket);
        WSACleanup();
        return NULL;
    }
    return socket;
}

bool bind_socket(Socket* socket, const char* address, int port) {
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address);
    server_address.sin_port = htons(port);

    return ::bind(socket->socket_fd, (sockaddr*)&server_address, sizeof(server_address)) == 0;
}

bool listen_socket(Socket* socket, int backlog) {
    return ::listen(socket->socket_fd, backlog) == 0;
}

Socket* accept_socket(Socket* socket) {
    sockaddr_in client_address;
    int client_address_size = sizeof(client_address);
    int client_socket_fd = ::accept(socket->socket_fd, (sockaddr*)&client_address, &client_address_size);

    if (client_socket_fd == INVALID_SOCKET) {
        return NULL;
    }

    Socket* client_socket = (Socket*)malloc(sizeof(Socket));
    client_socket->socket_fd = client_socket_fd;
    return client_socket;
}

bool connect_socket(Socket* socket, const char* address, int port) {
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address);
    server_address.sin_port = htons(port);

    return ::connect(socket->socket_fd, (sockaddr*)&server_address, sizeof(server_address)) == 0;
}

int send_socket(Socket* socket, const char* data, size_t size) {
    return ::send(socket->socket_fd, data, size, 0);
}

int receive_socket(Socket* socket, char* buffer, size_t size) {
    return ::recv(socket->socket_fd, buffer, size, 0);
}

bool close_socket(Socket* socket) {
    return ::closesocket(socket->socket_fd) == 0;
}

void free_socket(Socket* socket) {
    closesocket(socket->socket_fd);
    free(socket);
    WSACleanup();
}

#else

Socket* create_socket(int domain, int type, int protocol) {
    Socket* socket = (Socket*)malloc(sizeof(Socket));
    socket->socket_fd = ::socket(domain, type, protocol);
    if (socket->socket_fd == -1) {
        free(socket);
        return NULL;
    }
    return socket;
}

bool bind_socket(Socket* socket, const char* address, int port) {
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address);
    server_address.sin_port = htons(port);

    return ::bind(socket->socket_fd, (sockaddr*)&server_address, sizeof(server_address)) == 0;
}

bool listen_socket(Socket* socket, int backlog) {
    return ::listen(socket->socket_fd, backlog) == 0;
}

Socket* accept_socket(Socket* socket) {
    sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    int client_socket_fd = ::accept(socket->socket_fd, (sockaddr*)&client_address, &client_address_size);

    if (client_socket_fd == -1) {
        return NULL;
    }

    Socket* client_socket = (Socket*)malloc(sizeof(Socket));
    client_socket->socket_fd = client_socket_fd;
    return client_socket;
}

bool connect_socket(Socket* socket, const char* address, int port) {
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(address);
    server_address.sin_port = htons(port);

    return ::connect(socket->socket_fd, (sockaddr*)&server_address, sizeof(server_address)) == 0;
}

int send_socket(Socket* socket, const char* data, size_t size) {
    return ::send(socket->socket_fd, data, size, 0);
}

int receive_socket(Socket* socket, char* buffer, size_t size) {
    return ::recv(socket->socket_fd, buffer, size, 0);
}

bool close_socket(Socket* socket) {
    return ::close(socket->socket_fd) == 0;
}

void free_socket(Socket* socket) {
    close(socket->socket_fd);
    free(socket);
}

#endif