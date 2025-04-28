#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

class Epoll {
private:
    int _epoll_fd;
    std::vector<struct epoll_event> _events;

public:
    Epoll(size_t max_events = 10) : _events(max_events) {
        _epoll_fd = epoll_create(1); // paramètre ignoré
        if (_epoll_fd == -1)
            throw std::runtime_error("Failed to create epoll instance");
    }

    ~Epoll() {
        if (_epoll_fd != -1)
            close(_epoll_fd);
    }

    void add(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
            throw std::runtime_error("Failed to add fd to epoll");
    }

    void modify(int fd, uint32_t events) {
        struct epoll_event ev;
        ev.events = events;
        ev.data.fd = fd;
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == -1)
            throw std::runtime_error("Failed to modify fd in epoll");
    }

    void remove(int fd) {
        if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
            throw std::runtime_error("Failed to remove fd from epoll");
    }

    int wait(int timeout = -1) {
        int n = epoll_wait(_epoll_fd, &_events[0], _events.size(), timeout);
        if (n == -1)
            throw std::runtime_error("epoll_wait failed");
        return n;
    }

    struct epoll_event &get_event(int index) {
        return _events[index];
    }
};

#endif // EPOLL_HPP