#include "Socket.hpp"
#include <iostream>

int main()
{
    try
    {
        Socket serverSocket;

        serverSocket.create(AF_INET, SOCK_STREAM, 0);
        serverSocket.setReuseAddr();
        serverSocket.bind(8080);
        serverSocket.listen();
        //serverSocket.setNonBlocking();

        std::cout << "Server listening on port 8080..." << std::endl;

        struct sockaddr_in clientAddr;
        int clientFd = serverSocket.accept(clientAddr);

        std::cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;

        Socket clientSocket;
        clientSocket.setSockfd(clientFd);
        //clientSocket.setNonBlocking();

        std::string buffer;
        ssize_t bytes = clientSocket.recvData(buffer);
        if (bytes > 0)
        {
            std::cout << "Received: " << buffer << std::endl;
            clientSocket.sendData("Hello Client");
        }

        clientSocket.closeSocket();
        serverSocket.closeSocket();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}