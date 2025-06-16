#include "Socket.hpp"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

void acceptNewConnection(int listen_fd, int epoll_fd)
{
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        perror("accept");
        return;
    }

    // rendre la socket non bloquante si besoin

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // lecture, mode edge-triggered (optionnel)
    ev.data.fd = client_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl: client_fd");
        close(client_fd);
        return;
    }

    std::cout << "Nouvelle connexion acceptée : fd=" << client_fd << std::endl;
}