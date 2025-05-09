#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib> // atoi, strtol
#include <cstddef> // size_t

struct Location {
    std::string path;
    std::vector<std::string> methods;
    std::string root;
    std::string index;
    bool autoindex;
    bool upload_enable;
    std::string upload_store;
    std::string redirection;
    std::string cgi_extension;
    std::string cgi_path;
};

struct ServerConfig {
    std::string host;
	std::string root;
    std::vector<int> port;
    std::string server_names;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    std::vector<Location> routes;

    ServerConfig() :
        port(80),
        client_max_body_size(1000000) {}
};

void initializeLocation(Location a);

void initializeServerConfig(ServerConfig a);