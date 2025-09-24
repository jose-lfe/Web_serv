#ifndef STRUCT_HPP
#define STRUCT_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstddef>

struct ServerConfig;

struct Location {
    std::string path;
    std::vector<std::string> methods;
    std::string root;
    std::string index;
    size_t client_max_body_size;
    bool autoindex;
    bool upload_enable;
    std::string upload_store;
    std::string redirection;
    std::string cgi_extension;
    std::string cgi_path;

    Location();
    Location(const ServerConfig& config);
};

struct ServerConfig {
    char **cpy_envp;
    std::string host;
	std::string root;
    std::vector<int> port;
    std::vector<std::string> server_name;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    std::vector<Location> routes;
    std::string index;
    bool autoindex;

    ServerConfig();
};

#endif
