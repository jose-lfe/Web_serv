#include "Client.hpp"

	Client::Client() : _fd(-1), _keepAlive(true), _lastActivity(time(NULL)) {}
	Client::Client(int client_fd) : _fd(client_fd), _keepAlive(true), _lastActivity(time(NULL)) {}
	Client::~Client() {}