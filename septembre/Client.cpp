#include "Client.hpp"

	Client::Client() : _fd(-1), _keepAlive(true), _lastActivity(time(NULL)) {}
	Client::Client(int client_fd) : _fd(client_fd), _keepAlive(true), _lastActivity(time(NULL)) {}
	Client::~Client() {}

	void Client::resetAfterWrite()
	{
		_bufferIn.clear();
		_parsedRequest.method.clear();
		_parsedRequest.path.clear();
		_parsedRequest.http_version.clear();
		_parsedRequest.headers.clear();
		_parsedRequest.body.clear();
	}