// Socket.hpp
#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <vector>
#include <string>
#include <netinet/in.h>
#include "serverConfig.hpp"
#include "Struct.hpp"

class Socket {
public:
    Socket(const std::string &host, int port, std::vector<ServerConfig>& servers);
    ~Socket();

    bool bindAndListen();

    int getFd() const;
    int getPort() const;

private:
    // Désactive la copie
    Socket(const Socket& other);
    Socket& operator=(const Socket& other);

    int _fd;
    std::string _host;
    int _port;
    struct sockaddr_in _addr;
    std::vector<ServerConfig*> _servers;

    std::vector<ServerConfig*> init_servers(std::vector<ServerConfig>& servers);
};

#endif
