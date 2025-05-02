#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <string>
# include <vector>
# include "ServerConfig.hpp"

// Cette classe gère la config entière (fichier, parsing, etc.)
class Config {
public:
    ServerConfig server; // Contient la configuration d’un seul serveur (pas plusieurs)

    void parse(const std::string& filepath); // Fonction qui lit un fichier .conf
};

#endif