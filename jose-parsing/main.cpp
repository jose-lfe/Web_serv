#include "Config.hpp"
#include <iostream>

int main()
{
    Config config;
    config.parse("test.conf");

    std::cout << "Server is listening on port: " << config.server.listen_port << std::endl;
    std::cout << "Server name: " << config.server.server_name << std::endl;
    for (size_t i = 0; i < config.server.locations.size(); ++i) {
        std::cout << "Location: " << config.server.locations[i].path
                  << ", root = " << config.server.locations[i].root << std::endl;
    }

    return 0;
}

// g++ -std=c++98 main.cpp Config.cpp -o webserv