#include "Socket.hpp"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

Socket::Socket(const std::string &host, int port)
    : _fd(-1), _host(host), _port(port) {
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(_host.c_str());
    _addr.sin_port = htons(_port);
}

Socket::~Socket() {
    if (_fd >= 0)
        close(_fd);
}

bool Socket::bindAndListen() {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return false;
    }

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed\n";
        return false;
    }

    if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        std::cerr << "Bind failed on port " << _port << "\n";
        return false;
    }

    if (listen(_fd, 10) < 0) {
        std::cerr << "Listen failed on port " << _port << "\n";
        return false;
    }

    std::cout << "Listening on " << _host << ":" << _port << "\n";
    return true;
}

int Socket::getFd() const { return _fd; }
int Socket::getPort() const { return _port; }