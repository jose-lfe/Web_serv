#include "Socket.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>

Socket::Socket(const std::string &host, int port, std::vector<ServerConfig>& servers)
    : _fd(-1), _host(host), _port(port) {
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    if (_host.empty() || _host == "localhost") {
        _addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0
    } else {
        _addr.sin_addr.s_addr = inet_addr(_host.c_str());
        if (_addr.sin_addr.s_addr == INADDR_NONE) {
            std::cerr << "Warning: invalid IP address '" << _host << "', defaulting to INADDR_ANY\n";
            _addr.sin_addr.s_addr = INADDR_ANY;
        }
    }
    _servers = init_servers(servers);
    _addr.sin_port = htons(_port);

    std::cout << "Binding on IP: " << inet_ntoa(_addr.sin_addr) << " Port: " << _port << std::endl;
}

std::vector<ServerConfig*> Socket::init_servers(std::vector<ServerConfig>& servers) {
    std::vector<ServerConfig*> result;
    for (size_t i = 0; i < servers.size(); ++i) {
        ServerConfig& conf = servers[i];
        // Vérifie si ce serveur écoute sur ce port ET ce host (ou host vide/"0.0.0.0")
        for (size_t j = 0; j < conf.port.size(); ++j) {
            bool port_match = (conf.port[j] == _port);
            bool host_match = (conf.host.empty() || conf.host == "0.0.0.0" || conf.host == _host);
            if (port_match && host_match) {
                result.push_back(&conf);
                break; // Un serveur ne doit être ajouté qu'une fois
            }
        }
    }
    return result;
}

Socket::~Socket() {
    std::cout << "[~Socket] Destroying fd = " << _fd << std::endl;
    if (_fd >= 0) {
        close(_fd);
    }
}


Socket::Socket(Socket&& other) noexcept
    : _fd(other._fd), _host(std::move(other._host)), _port(other._port), _addr(other._addr) {
    other._fd = -1; // Invalide le fd de l'ancien socket
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (_fd >= 0) close(_fd);
        _fd = other._fd;
        _host = std::move(other._host);
        _port = other._port;
        _addr = other._addr;
        other._fd = -1; // Invalide l'ancien
    }
    return *this;
}


bool Socket::bindAndListen() {
    std::cout << "salut" << std::endl;
    _fd = socket(AF_INET, SOCK_STREAM, 0);
std::cout << "Created socket, fd = " << _fd << std::endl;

    if (_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return false;
    }
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1) { /* erreur */ }
    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1) { /* erreur */ }


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