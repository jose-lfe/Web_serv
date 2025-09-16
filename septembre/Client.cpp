#include "Client.hpp"

	Client::Client() : _fd(-1), _keepAlive(true), _port(0), _lastActivity(time(NULL)) {}
	Client::Client(int client_fd, int port) : _fd(client_fd), _keepAlive(true), _port(port), _lastActivity(time(NULL)) {}
	Client::~Client() {}