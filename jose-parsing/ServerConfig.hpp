#ifndef SERVER_CONFIG_HPP
# define SERVER_CONFIG_HPP

# include <string>
# include <vector>
# include "LocationConfig.hpp"

// Cette structure représente la configuration d’un serveur HTTP complet
struct ServerConfig {
    int listen_port;                      // Port d'écoute du serveur (80, 8080...)
    std::string server_name;              // Nom du serveur (optionnel mais utile)
    std::vector<LocationConfig> locations; // Les "location {...}" du fichier de config

    // Constructeur par défaut qui fixe le port à 80
    ServerConfig() : listen_port(80) {}
};

#endif