#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ctime>
#include <string>
#include "handleRequest.hpp"
#include <deque>
class Client {

public:
	Client();
	Client(int client_fd, int port);
	~Client();
	int _fd;
	bool _keepAlive;
	int _port;
	time_t _lastActivity;
	std::string _bufferIn;
	std::deque<std::string> _responseQueue;
	handleRequest _parsedRequest;
};

#endif