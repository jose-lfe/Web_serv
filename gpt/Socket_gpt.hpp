#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cstring>

class Socket {
private:
    int _fd;

public:
    Socket() : _fd(-1) {std::cout << "Client in" << std::endl;}

    Socket(int fd) : _fd(fd) {{std::cout << "Client 2 in" << std::endl;}}

    ~Socket() {
        std::cout << "Client out " << _fd << std::endl;
        if (_fd != -1)
            close(_fd);
    }

    void create(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0) {
        _fd = socket(domain, type, protocol);
        if (_fd == -1)
            throw std::runtime_error("Failed to create socket");
    }

    void bind(const char *ip, int port) {
        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);

        if (::bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
            throw std::runtime_error("Failed to bind socket");
    }

    void listen(int backlog = 10) {
        if (::listen(_fd, backlog) == -1)
            throw std::runtime_error("Failed to listen on socket");
    }

    Socket accept() {
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        int client_fd = ::accept(_fd, (struct sockaddr*)&addr, &addrlen);
        if (client_fd == -1)
            throw std::runtime_error("Failed to accept connection");
        return Socket(client_fd);
    }

    ssize_t recv(char *buffer, size_t length) {
        return ::recv(_fd, buffer, length, 0);
    }

    ssize_t send(const char *buffer, size_t length) {
        return ::send(_fd, buffer, length, 0);
    }

    void close_socket() {
        if (_fd != -1) {
            close(_fd);
            _fd = -1;
        }
    }

    int get_fd() const { return _fd; }
};

#endif // SOCKET_HPP