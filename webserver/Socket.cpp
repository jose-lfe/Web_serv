#include "Socket.hpp"
#include <cstring>
#include <cerrno>
#include <iostream>

Socket::Socket() : _sockfd(-1)
{
    std::memset(&_addr, 0, sizeof(_addr));
}

Socket::~Socket()
{
    if (_sockfd != -1)
        close(_sockfd);
}

void Socket::create(int domain, int type, int protocol)
{
    _sockfd = socket(domain, type, protocol);
    if (_sockfd == -1)
        throw std::runtime_error("Socket creation failed");
}

void Socket::setNonBlocking()
{
    int flags = fcntl(_sockfd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl F_GETFL failed");

    if (fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed");
}

void Socket::setReuseAddr()
{
    int opt = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
}

void Socket::bind (int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(port);

    if (::bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1)
        throw std::runtime_error("Bind failed");
}

void Socket::listen(int backlog)
{
    if (::listen(_sockfd, backlog) == -1)
        throw std::runtime_error("Listen failed");
}

int Socket::accept(struct sockaddr_in &clientAddr)
{
    socklen_t addrLen = sizeof(clientAddr);
    int clientFd = ::accept(_sockfd, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientFd == -1)
        throw std::runtime_error("Accept failed");

    return clientFd;
}

void Socket::connect(const std::string &ip, int port)
{
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &_addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid address/Address not supported");
    if (::connect(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) < 0)
        throw std::runtime_error("Connection failed");
}

ssize_t Socket::sendData(const std::string &data)
{
    return ::send(_sockfd, data.c_str(), data.size(), 0);
}

ssize_t Socket::recvData(std::string &buffer)
{
    char tempBuf[1024];
    ssize_t bytes = ::recv(_sockfd, tempBuf, sizeof(tempBuf) - 1, 0);
    if (bytes > 0)
    {
        tempBuf[bytes] = '\0';
        buffer = tempBuf;
    }
    return bytes;
}

void Socket::closeSocket()
{
    if (_sockfd != -1)
    {
        ::close(_sockfd);
        _sockfd = -1;
    }
}

int Socket::getSockfd() const
{
    return _sockfd;
}

void Socket::setSockfd(int fd)
{
    _sockfd = fd;
}