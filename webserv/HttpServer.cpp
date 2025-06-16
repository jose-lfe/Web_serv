#include "HttpServer.hpp"
#include "Socket.hpp"
#include "HandleRequest.hpp"
#include "SimpleRouter.hpp"
#include <string>
#include <map>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include <set>

HttpServer::HttpServer(const std::vector<ServerConfig> &configs) : _configs(configs), _epoll_fd(-1) {}

HttpServer::~HttpServer() {}

bool HttpServer::setupSockets()
{
    std::set<std::pair<std::string, int> > opened;

    for (size_t i = 0; i < _configs.size(); ++i)
    {
        const ServerConfig &conf = _configs[i];

        for (size_t j = 0; j < conf.port.size(); ++j) {
            int port = conf.port[j];
            std::pair<std::string, int> key = std::make_pair(conf.host, port);

            if (opened.count(key))
                continue;

            Socket sock(conf.host, port, _configs);
            if (!sock.bindAndListen())
                return false;

            _listenSockets.push_back(std::move(sock));
            opened.insert(key);
        }
    }
    return true;
}

// bool HttpServer::setupSockets()
// {
// 	std::map<int, bool> openedPorts;

// 	for (size_t i = 0; i < _configs.size(); ++i)
// 	{
//         const ServerConfig &conf = _configs[i];

	
// 		for (size_t j = 0; j < conf.port.size(); ++j) {
// 			int port = conf.port[j];

// 			if (openedPorts[port])
// 				continue;

// 			Socket sock(conf.host, port, _configs);;
// 			if (!sock.bindAndListen())
// 				return false;

// 			_listenSockets.push_back(std::move(sock));
//     	    openedPorts[port] = true;
// 		}
// 	}
// 	return true;
// }

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
    for (const Socket& sock : _listenSockets) {
        int sockfd = sock.getFd();
        std::cout << "Adding listen socket fd: " << sockfd << std::endl;

        if (sockfd < 0) {
            std::cerr << "Invalid listen socket fd\n";
            exit(1);
        }

        // Optionnel: s'assurer que la socket est non bloquante
        int flags = fcntl(sockfd, F_GETFL, 0);
        if (flags == -1) {
            perror("fcntl F_GETFL");
            exit(1);
        }
        if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl F_SETFL O_NONBLOCK");
            exit(1);
        }

        epoll_event ev{};
        ev.events = EPOLLIN;  // Prêt à lire sur socket d'écoute
        ev.data.fd = sockfd;

        std::cout << "epoll_ctl add: fd=" << sockfd << " events=" << ev.events << std::endl;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
            perror("epoll_ctl: listen socket");
            exit(1);
        }
    }

    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (true) {
        int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n_events == -1) {
            perror("epoll_wait");
            continue;
        }

        for (int i = 0; i < n_events; ++i) {
            int fd = events[i].data.fd;

            if (isListenSocket(fd)) {
                acceptClient(fd, epoll_fd);
            } else {
                if (events[i].events & EPOLLIN) {
                    handleRead(fd, epoll_fd);
                }
                if (events[i].events & EPOLLOUT) {
                    handleWrite(fd, epoll_fd);
                }
                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    closeClient(fd, epoll_fd);
                }
            }
        }
    }

    close(epoll_fd);
}

void HttpServer::handleRead(int fd, int epoll_fd) {
    char buffer[4096];
    ssize_t bytesRead;
    bool gotData = false;

    // Lire en boucle tant que recv > 0 (EPOLLET)
    std::cout << "Lecture des données sur fd=" << fd << std::endl; // debug
    while ((bytesRead = recv(fd, buffer, sizeof(buffer), 0)) > 0) {
        _readBuffers[fd] += std::string(buffer, bytesRead);
        std::cout << "Reçu " << bytesRead << " octets sur fd=" << fd << std::endl; // debug
        gotData = true;
    }
    std::cout << "Buffer reçu (fd=" << fd << ") :\n" << _readBuffers[fd] << std::endl; // debug

    if (!gotData && (bytesRead == 0 || (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK))) {
        std::cerr << "Client closed connection or error on fd=" << fd << std::endl;
        closeClient(fd, epoll_fd);
        return;
    }

    // Cherche la fin des headers
    size_t headers_end = _readBuffers[fd].find("\r\n\r\n");
    if (headers_end == std::string::npos)
        return; // On n'a pas encore tous les headers

    // Cherche Content-Length
    size_t content_length = 0;
    std::string headers = _readBuffers[fd].substr(0, headers_end);
    std::istringstream hstream(headers);
    std::string line;
    while (std::getline(hstream, line)) {
        if (line.find("Content-Length:") == 0) {
            std::string value = line.substr(15);
            value.erase(0, value.find_first_not_of(" \t"));
            content_length = std::atoi(value.c_str());
            break;
        }
    }

    size_t total_needed = headers_end + 4 + content_length;
    // Si Content-Length > 0, attendre tout le body
    if (content_length > 0 && _readBuffers[fd].size() < total_needed)
        return; // On n'a pas encore tout le body

    // On a tout reçu : headers + body (ou juste headers pour GET)
    std::string full_request = _readBuffers[fd].substr(0, total_needed);

    std::cout << "Requête HTTP complète reçue depuis fd=" << fd << " :\n"
              << full_request << std::endl;

    HandleRequest req;
    if (req.parse(full_request)) {
        _parsedRequests[fd] = req;
    } else {
        std::cerr << "Failed to parse HTTP request on fd=" << fd << std::endl;
        closeClient(fd, epoll_fd);
        return;
    }

    req.print();

    // Passe en mode écriture
    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        perror("epoll_ctl: mod to EPOLLOUT");
        closeClient(fd, epoll_fd);
    }

    // Nettoie le buffer pour ce fd
    _readBuffers.erase(fd);
}


// void HttpServer::handleRead(int fd, int epoll_fd) {
//     char buffer[4096];
//     ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);

//     if (bytesRead <= 0) {
//         std::cerr << "Client closed connection or error on fd=" << fd << std::endl;
//         closeClient(fd, epoll_fd);
//         return;
//     }

//     _readBuffers[fd] += std::string(buffer, bytesRead);

//     // Vérifie si on a reçu toute la requête (en-têtes terminés)
//     if (_readBuffers[fd].find("\r\n\r\n") != std::string::npos) {
//         std::cout << "Requête HTTP reçue depuis fd=" << fd << " :\n"
//                   << _readBuffers[fd] << std::endl;

// 		HandleRequest req;
// 		if (req.parse(_readBuffers[fd])) {
// 			_parsedRequests[fd] = req;
// 		}
// 		else {
// 			std::cerr << "Failed to parse HTTP request on fd=" << fd << std::endl;
// 			closeClient(fd, epoll_fd);
// 			return ;
// 		}

// 		req.print();
//         // ⚠️ À FAIRE : ici on ajoutera parseHeaders plus tard

//         // Passe en mode écriture
//         epoll_event ev;
//         ev.events = EPOLLOUT;
//         ev.data.fd = fd;
//         if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) < 0) {
//             perror("epoll_ctl: mod to EPOLLOUT");
//             closeClient(fd, epoll_fd);
//         }
//     }
// }

void HttpServer::closeClient(int fd, int epoll_fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    _readBuffers.erase(fd);
    std::cout << "Connexion fermée : fd=" << fd << std::endl;
}

bool HttpServer::isListenSocket(int fd) const {
    for (const Socket& sock : _listenSockets) {
        if (sock.getFd() == fd)
            return true;
    }
    return false;
}

void HttpServer::acceptClient(int listen_fd, int epoll_fd) {
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    // Optionnel : rendre non-bloquant ici (tu peux ajouter ça plus tard)
    if (!setNonBlocking(client_fd)) {
    perror("fcntl client_fd O_NONBLOCK");
    close(client_fd);
    return;
}

    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // lecture, mode edge-triggered
    ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl: client_fd");
        close(client_fd);
        return;
    }

    std::cout << "Nouvelle connexion acceptée : fd=" << client_fd << std::endl;
}

void HttpServer::handleWrite(int fd, int epoll_fd) {
	std::map<int, HandleRequest>::iterator it = _parsedRequests.find(fd);
	if (it == _parsedRequests.end()) {
		std::cerr << "No parsed request for fd=" << fd << std::endl;
		closeClient(fd, epoll_fd);
		return;
	}

	const HandleRequest& req = it->second;
	std::string response = SimpleRouter::route(req, _configs);

	size_t totalSent = 0;
	size_t toSend = response.size();

	while (totalSent < toSend) {
	ssize_t bytesSent = send(fd, response.c_str() + totalSent, toSend - totalSent, 0);
	if (bytesSent < 0) {
		perror("send");
		closeClient(fd, epoll_fd);
		return;
	}
	totalSent += bytesSent;
	}

	closeClient(fd, epoll_fd);
	_parsedRequests.erase(it);
}

// void HttpServer::handleWrite(int fd, int epoll_fd) {
// 	std::map<int, HandleRequest>::iterator it = _parsedRequests.find(fd);
// 	if (it == _parsedRequests.end()) {
// 		std::cerr << "No parsed request for fd=" << fd << std::endl;
// 		closeClient(fd, epoll_fd);
// 		return;
// 	}

// 	const HandleRequest& req = it->second;
// 	std::string response = SimpleRouter::route(req.method, req.path);

// 	size_t totalSent = 0;
// 	size_t toSend = response.size();

// 	while (totalSent < toSend) {
// 	ssize_t bytesSent = send(fd, response.c_str() + totalSent, toSend - totalSent, 0);
// 	if (bytesSent < 0) {
// 		perror("send");
// 		closeClient(fd, epoll_fd);
// 		return;
// 	}
// 	totalSent += bytesSent;
// 	}

// 	closeClient(fd, epoll_fd);
// 	_parsedRequests.erase(it);
// }

