#include "HttpServer.hpp"
#include "Socket.hpp"
#include "handleRequest.hpp"
#include "SimpleRouter.hpp"
#include <string>
#include <map>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include "Client.hpp"
#include <set>

HttpServer::HttpServer(const std::vector<ServerConfig> &configs) : _configs(configs) {}

HttpServer::~HttpServer() {}

void HttpServer::setupSockets()
{
    std::set<std::pair<std::string, int> > opened;

    for (size_t i = 0; i < _configs.size(); ++i)
    {
        const ServerConfig &conf = _configs[i];

        for (size_t j = 0; j < conf.port.size(); ++j) {
            int port = conf.port[j];
            std::pair<std::string, int> key = std::make_pair(conf.host, port);

            if (opened.count(key)) {
                std::cout << "Already opened socket for " << conf.host << ":" << port << ", skipping" << std::endl;
                continue;
            }

            std::cout << "Creating socket on " << conf.host << ":" << port << std::endl;
            Socket* sock = new Socket(conf.host, port, _configs);

			try {sock->bindAndListen();}
			catch(std::exception &e)
			{
				std::cerr << e.what();
				delete sock; // éviter fuite mémoire
				throw std::runtime_error("Error: failed bindAndListen");
			}

            std::cout << "Socket created and listening on fd = " << sock->getFd() << std::endl;

            _listenSockets.push_back(sock);
            opened.insert(key);
        }
    }
    return;
}

#include <fcntl.h>

bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags |= O_NONBLOCK;
    return (fcntl(fd, F_SETFL, flags) != -1);
}


void HttpServer::run()
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }
    std::cout << "epoll_fd = " << epoll_fd << std::endl;
    std::cout << "_listenSockets size: " << _listenSockets.size() << std::endl;
    for (std::vector<Socket*>::const_iterator it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
	    Socket* sock = *it;
        int sockfd = sock->getFd();
        std::cout << "Adding listen socket fd: " << sockfd << std::endl;

        if (sockfd < 0) {
            throw std::runtime_error("Error: invalid listen socket fd");
        }

        // Rend non bloquant
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1) {
            throw std::runtime_error("Error: fcntl F_GETFL failed");
        }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            throw std::runtime_error("Error: fcntl F_SETFL O_NONBLOCK failed");
        }

        epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;  // Prêt à lire sur socket d'écoute
        ev.data.fd = sockfd;

        std::cout << "epoll_ctl add: fd=" << sockfd << " events=" << ev.events << std::endl;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
            throw std::runtime_error("Error: epoll_ctl: listen socket failed");
        }
    }

    const int MAX_EVENTS = 64;
	const int TIMEOUT_SECONDS = 3;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_events == -1) {
            perror("epoll_wait");
            continue;
        }

		time_t now = time(NULL);
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); )
		{
			if (difftime(now, it->second._lastActivity) > TIMEOUT_SECONDS)
			{
				std::cout << "Timeout, closing fd=" << it->first << std::endl;
				int fd = it->first;
				++it;
				closeClient(fd, epoll_fd);
			}
			else
			++it;
		}

        for (int i = 0; i < n_events; ++i) {
            int fd = events[i].data.fd;

            if (isListenSocket(fd)) {
				int targetPort = 0;
				for (size_t i = 0; i < _listenSockets.size(); ++i)
					if (_listenSockets[i]->getFd() == fd)
					{
						targetPort = _listenSockets[i]->getPort(); 
					}
                acceptClient(fd, epoll_fd, targetPort);
				continue;
            }
			std::map<int, Client>::iterator it = _clients.find(fd);
			if (it == _clients.end())
			{
				std::cerr << "Client fd not found: " << fd << std::endl;
				continue;
			}
			Client &client = it->second;
            if (events[i].events & EPOLLIN) {
                handleRead(fd, epoll_fd, client);
            }
            if (events[i].events & EPOLLOUT) {
                handleWrite(fd, epoll_fd, client);
            }
            if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                std::cout << "closing bc EPOLLHUP" << std::endl;
                closeClient(fd, epoll_fd);
            }
            }
        }

    close(epoll_fd);
}

void HttpServer::handleRead(int fd, int epoll_fd, Client &client) {
	
	char buffer[4096];
    ssize_t bytesRead;

    std::cout << "Reading data on fd=" << fd << std::endl; // debug
    bytesRead = recv(fd, buffer, sizeof(buffer), 0);
    
    if (bytesRead > 0)
    {
        client._bufferIn.append(buffer, bytesRead);
		client._lastActivity = time(NULL);
        std::cout << "Received " << bytesRead << " bytes on fd=" << fd << std::endl; // debug
    }

    else if (bytesRead == 0) {
        std::cerr << "Client closed connection on fd=" << fd << std::endl;
        closeClient(fd, epoll_fd);
        return;
    }
    else if (bytesRead == -1)
    {
        return;
    }
    
	if (client._bufferIn.empty())
		return;

    size_t headers_end = client._bufferIn.find("\r\n\r\n");
    if (headers_end == std::string::npos)
        return;

    std::string headers = client._bufferIn.substr(0, headers_end);
    bool is_chunked = false;
    size_t content_length = 0;

    std::istringstream hstream(headers);
    std::string line;
    while (std::getline(hstream, line)) {
        if (line.find("Transfer-Encoding:") == 0 && line.find("chunked") != std::string::npos) {
            is_chunked = true;
        }
        if (line.find("Content-Length:") == 0) {
            std::string value = line.substr(15);
            value.erase(0, value.find_first_not_of(" \t"));
            content_length = std::atoi(value.c_str());
        }
    }

    std::string full_request;
    if (is_chunked)
		full_request = headers + "\r\n\r\n";
	else
	{
		size_t total_needed = headers_end + 4 + content_length;
		if (client._bufferIn.size() < total_needed)
			return;
		full_request = client._bufferIn.substr(0, total_needed);
	}

    handleRequest req;
    if (!req.parse(full_request)) {
		std::cout << "closing bc req parse" << std::endl;
        closeClient(fd, epoll_fd);
		return;
    }
	client._parsedRequest = req;

	std::string response = SimpleRouter::route(req, _configs, client._port);
	client._responseQueue.push_back(response);

    req.print();

    epoll_event ev;
    ev.events = EPOLLET | EPOLLIN | EPOLLOUT;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        perror("epoll_ctl: mod to EPOLLOUT");
            std::cout << "closing bc epoll < 0" << std::endl;
        closeClient(fd, epoll_fd);
    }

    client._bufferIn.erase(0, full_request.size());
}


void HttpServer::closeClient(int fd, int epoll_fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    _clients.erase(fd);
    std::cout << "Connection closed : fd=" << fd << std::endl;
}

bool HttpServer::isListenSocket(int fd) const {
	std::vector<Socket*>::const_iterator it;
	for (it = _listenSockets.begin(); it != _listenSockets.end(); ++it) {
		if ((*it)->getFd() == fd)
			return true;
	}
	return false;
}

void HttpServer::acceptClient(int listen_fd, int epoll_fd, int port) {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    if (!setNonBlocking(client_fd)) {
    perror("fcntl client_fd O_NONBLOCK");
    close(client_fd);
    return;
}
	_clients[client_fd] = Client(client_fd, port);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl: client_fd");
        close(client_fd);
		_clients.erase(client_fd);
        return;
    }

    std::cout << "New connection accepted : fd=" << client_fd << std::endl;
}

void HttpServer::deleteSockets()
{
    for (size_t i = 0; i < _listenSockets.size(); ++i) {
    delete _listenSockets[i];
}
}


void HttpServer::handleWrite(int fd, int epoll_fd, Client &client) {

	std::cout << "starting handlewrite" << std::endl;
    
	while (!client._responseQueue.empty())
	{
		std::string &response = client._responseQueue.front();
		ssize_t bytesSent = send(fd, response.c_str(), response.size(), 0);

		if (bytesSent <= 0)
		{
			return;
		}
		client._lastActivity = time(NULL);
		response.erase(0, bytesSent);

		if (response.empty())
			client._responseQueue.pop_front();
		else
			return;
	}

	if (!client._keepAlive)
	{
        std::cout << "closing bc !keepAlive" << std::endl;
		closeClient(fd, epoll_fd);
		return;
	}
}

