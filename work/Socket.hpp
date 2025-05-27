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

	// Disable copy
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
	
	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;

	bool bindAndListen();
	int getFd() const;
	int getPort() const;

};

#endif