#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <vector>
#include <iostream>
#include "Socket.hpp"
#include "Struct.hpp"
#include "handleRequest.hpp"
#include "Client.hpp"

class HttpServer
{
private:
	std::vector<ServerConfig> _configs;
	std::vector<Socket*> _listenSockets;
	std::map<int, handleRequest> _parsedRequests;
	std::map<int, Client> _clients;

public:
	HttpServer();
	HttpServer& operator=(const HttpServer &other);
	HttpServer(const std::vector<ServerConfig> &configs);
	~HttpServer();

	void setupSockets();
	void handleRead(int fd, int epoll_fd, Client &client);
	void handleWrite(int fd, int epoll_fd, Client &client);
	void closeClient(int fd, int epoll_fd);
	bool isListenSocket(int fd) const;
	void acceptClient(int listen_fd, int epoll_fd, int port);
	void deleteSockets();


	void run();

};

#endif
