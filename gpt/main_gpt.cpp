#include "Socket_gpt.hpp"
#include "Epoll_gpt.hpp"
#include <map>
#include <iostream>

int main() {
    try {
        Socket server;
        server.create();
        server.bind("0.0.0.0", 8080);
        server.listen();

        Epoll poller;
        poller.add(server.get_fd(), EPOLLIN);

        std::map<int, Socket> clients;

        while (true) {
            int n = poller.wait();
            for (int i = 0; i < n; ++i) {
                struct epoll_event &ev = poller.get_event(i);

                if (ev.data.fd == server.get_fd()) {
                    // Nouvelle connexion
                    Socket client = server.accept();
                    int client_fd = client.get_fd();
                    poller.add(client_fd, EPOLLIN);
                    clients.insert(std::make_pair(client_fd, client));
                    std::cout << "Nouveau client connecté, fd: " << client_fd << std::endl;
                } else {
                    // Socket client prêt à lire
                    char buffer[4096];
                    ssize_t bytes = clients[ev.data.fd].recv(buffer, sizeof(buffer) - 1);

                    if (bytes <= 0) {
                        // Déconnexion
                        poller.remove(ev.data.fd);
                        clients[ev.data.fd].close_socket();
                        clients.erase(ev.data.fd);
                        std::cout << "Client déconnecté, fd: " << ev.data.fd << std::endl;
                    } else {
                        buffer[bytes] = '\0'; // Terminer proprement la chaîne
                        std::cout << "Reçu : " << buffer << std::endl;

                        // ✨ Simple réponse HTTP ✨
                        const char response[] =
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: 12\r\n"
                            "\r\n"
                            "Hello World";

                        std::cout << "Envoi de la réponse HTTP : " << response << std::endl;
                        clients[ev.data.fd].send(response, sizeof(response) - 1);
                        
                        // Si tu veux, tu peux aussi fermer le client juste après en HTTP/1.0
                        poller.remove(ev.data.fd);
                        clients[ev.data.fd].close_socket();
                        clients.erase(ev.data.fd);
                    }
                }
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
    }

    return 0;
}
