#ifndef LOCATION_CONFIG_HPP
# define LOCATION_CONFIG_HPP

# include <string>
# include <vector>

struct LocationConfig {
    std::string path;
    std::string root;

    LocationConfig() {} // constructeur par default (meme pour les structs)
};

#endif