#include "HttpServer.hpp"
#include "Socket.hpp"
#include <string>
#include <map>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

HttpServer::HttpServer(const std::vector<ServerConfig> &configs) : _configs(configs) {}

bool HttpServer::setupSockets()
{
	std::map<int, bool> openedPorts;

	for (size_t i = 0; i < _configs.size(); ++i)
	{
        const ServerConfig &conf = _configs[i];

	
		for (size_t j = 0; j < conf.port.size(); ++j) {
			int port = conf.port[j];

			if (openedPorts[port])
				continue;

			Socket sock(conf.host, port);;
			if (!sock.bindAndListen())
				return false;

			_listenSockets.push_back(sock.getFd());
    	    openedPorts[port] = true;
		}
	}
	return true;
}

void HttpServer::run()
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    // Ajouter tous les sockets d'écoute à epoll
    for (int sockfd : _listenSockets) {
        epoll_event ev{};
        ev.events = EPOLLIN; // prêt à lire
        ev.data.fd = sockfd;

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
                // Nouvelle connexion
                acceptClient(fd, epoll_fd);
            } else {
                if (events[i].events & EPOLLIN) {
                    handleRead(fd, epoll_fd);  // lecture
                }
                if (events[i].events & EPOLLOUT) {
                    handleWrite(fd, epoll_fd); // écriture
                }
                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    closeClient(fd, epoll_fd); // erreur / fermeture
                }
            }
        }
    }

    close(epoll_fd);
}