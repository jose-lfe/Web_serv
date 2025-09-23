#include "Socket.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

Socket::Socket(const std::string &host, int port, std::vector<ServerConfig>& servers)
    : _fd(-1), _host(host), _port(port) {
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;

    _addr.sin_addr.s_addr = resolveHost(_host);
    _addr.sin_port = htons(_port);
	_servers = init_servers(servers);

    std::cout << "Binding on IP: " << inet_ntoa(_addr.sin_addr) << " Port: " << _port << std::endl;
}

Socket::~Socket() {
	if (_fd >= 0) {
		close(_fd);
		std::cout << "[~Socket] Destroying fd = " << _fd << std::endl;
    }
}

std::vector<ServerConfig*> Socket::init_servers(std::vector<ServerConfig>& servers) {
    std::vector<ServerConfig*> result;
    for (size_t i = 0; i < servers.size(); ++i) {
        ServerConfig& conf = servers[i];
        for (size_t j = 0; j < conf.port.size(); ++j) {
            bool port_match = (conf.port[j] == _port);
            bool host_match = (conf.host.empty() || conf.host == "0.0.0.0" || conf.host == _host);
            if (port_match && host_match) {
                result.push_back(&conf);
                break;
            }
        }
    }
    return result;
}

void Socket::bindAndListen() {
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Created socket, fd = " << _fd << std::endl;

    if (_fd < 0) {
        throw std::runtime_error("Error: socket creation failed");
    }

    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
	{
		throw std::runtime_error("Error: socket nonblock flag failed");
	}
    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Error: socket nonblog flag set failed");
	}

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Error: setsockopt failed");
    }

    if (bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        throw std::runtime_error("Error: socket bind failed");
    }

    if (listen(_fd, 10) < 0) {
        throw std::runtime_error("Error: socket listen failed");
    }

    std::cout << "Listening on " << _host << ":" << _port << std::endl;
	return;
}

int Socket::getFd() const {return _fd;}
int Socket::getPort() const {return _port;}

in_addr_t Socket::resolveHost(const std::string &host)
{
	if (host.empty() || host == "localhost")
	{
		return INADDR_ANY;
	}
	in_addr_t addr = inet_addr(host.c_str());
	if (addr == INADDR_NONE)
	{
		std::cout << "Warning invalid IP '" + host + "', defaulting to INADDR_ANY" << std::endl;
		return INADDR_ANY;
	}
	return addr;
}
