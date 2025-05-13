#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket
{
private:
	int _fd;
	std::string _host;
	int _port;
	sockaddr_in _addr;

public:
	Socket(const std::string &host, int port);
	~Socket();

	bool bindAndListen();
	int getFd() const;
	int getPort() const;

};

#endif