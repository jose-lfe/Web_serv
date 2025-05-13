#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <vector>
#include <iostream>
#include "serverConfig.hpp"
#include "Struct.hpp"

class HttpServer
{
private:
	std::vector<ServerConfig> _configs;
	std::vector<int> _listenSockets;

public:
	HttpServer(const std::vector<ServerConfig> &configs);
	~HttpServer();

	bool setupSockets();
	void run();
};

#endif