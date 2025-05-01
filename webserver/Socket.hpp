#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <stdexcept>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


class Socket {
private:
    int _sockfd;
    struct sockaddr_in _addr;

public:
// Constructeurs
Socket();
~Socket();

// Methodes
void create(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
void setNonBlocking();
void setReuseAddr();
void bind(int port);
void listen(int backlog = 128);
int accept(struct sockaddr_in &clientAddr);
void connect(const std::string &data, int port);
ssize_t sendData(const std::string &data);
ssize_t recvData(std::string &buffer);
void closeSocket();

int getSockfd() const;
void setSockfd(int fd);
};

#endif