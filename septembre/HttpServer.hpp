#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <vector>
#include <iostream>
#include "Socket.hpp"
#include "Struct.hpp"
#include "handleRequest.hpp"
#include "Client.hpp"
//#include <unordered_map>

class HttpServer
{
private:
	std::vector<ServerConfig> _configs;
	std::vector<Socket*> _listenSockets;
	std::map<int, handleRequest> _parsedRequests;
	std::map<int, Client> _clients;

public:
	HttpServer(const std::vector<ServerConfig> &configs);
	~HttpServer();

	bool setupSockets();
	void handleRead(int fd, int epoll_fd, Client &client);
	void handleWrite(int fd, int epoll_fd, Client &client);
	void closeClient(int fd, int epoll_fd);
	bool isListenSocket(int fd) const;
	void acceptClient(int listen_fd, int epoll_fd);
	void deleteSockets();


	void run();

};

#endif
