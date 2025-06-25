#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include "Struct.hpp"

class Socket
{
private:
	int _fd;
	std::string _host;
	int _port;
	sockaddr_in _addr;
	std::vector<ServerConfig*> _servers;

public:
	Socket(const std::string &host, int port, std::vector<ServerConfig>& servers);
	~Socket();

	// Disable copy
    // Socket(const Socket&) = delete;
    // Socket& operator=(const Socket&) = delete;
	
	// Socket(Socket&& other);
	// Socket& operator=(Socket&& other);

	std::vector<ServerConfig*> init_servers(std::vector<ServerConfig>& servers);

	Socket(const Socket& other);
	Socket& operator=(const Socket& other);


	bool bindAndListen();
	int getFd() const;
	int getPort() const;

};

#endif