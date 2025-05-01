#include "Socket.hpp"
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 10

int main()
{
    try
    {
        Socket serverSocket;

        serverSocket.create(AF_INET, SOCK_STREAM, 0);
        serverSocket.setReuseAddr();
        serverSocket.setNonBlocking();
        serverSocket.bind(8080);
        serverSocket.listen(10);

        int epollFd = epoll_create1(0);
        if (epollFd == -1)
            throw std::runtime_error("epoll_create1 failed");

        epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = serverSocket.getSockfd();

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket.getSockfd(), &event) == -1)
            throw std::runtime_error("epoll_ctl failed");

        std::cout << "Server listening on port 8080..." << std::endl;

        epoll_event events[MAX_EVENTS];

        while (true)
        {
            int ready = epoll_wait(epollFd, events, MAX_EVENTS, -1);
            if (ready == -1)
            {
                if (errno == EINTR) continue;
                throw std::runtime_error("epoll_wait failed");
            }

            for (int i = 0; i < ready; ++i)
            {
                int fd = events[i].data.fd;

                if (fd == serverSocket.getSockfd())
                {
                    sockaddr_in clientAddr;
                    try
                    {
                        int clientFd = serverSocket.accept(clientAddr);
                        fcntl(clientFd, F_SETFL, fcntl(clientFd, F_GETFL, 0) | O_NONBLOCK);

                        epoll_event clientEvent;
                        clientEvent.events = EPOLLIN | EPOLLET;
                        clientEvent.data.fd = clientFd;

                        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &clientEvent) == -1)
                            throw std::runtime_error("epoll_ctl client failed");

                        std::cout << "New client (fd=" << clientFd << ")" << std::endl;
                    }
                    catch(...)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                            std::cerr << "Error accept: " << strerror(errno) << std::endl;
                    }
                }
                else
                {
                    char buf[1024];
                    ssize_t bytes = recv(fd, buf, sizeof(buf) - 1, 0);
                    if (bytes > 0)
                    {
                        buf[bytes] = '\0';
                        std::cout << "Client " << fd << " > " << buf;
                    }
                    else if (bytes == 0 || (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
                    {
                        std::cout << "Client " << fd << " disconnected." << std::endl;
                        close(fd);
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
                    }
                }
            }


        }
        close(epollFd);
        serverSocket.closeSocket();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}