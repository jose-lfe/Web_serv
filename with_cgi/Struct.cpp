#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib> // atoi, strtol
#include <cstddef> // size_t
#include "Struct.hpp"

// struct Location {
//     std::string path;
//     std::vector<std::string> methods;
//     std::string root;
//     std::string index;
//     bool autoindex;
//     bool upload_enable;
//     std::string upload_store;
//     std::string redirection;
//     std::string cgi_extension;
//     std::string cgi_path;


Location::Location() :
    path(""),
    root(""),
    index(""),
    autoindex(false),
    upload_enable(false),
    upload_store(""),
    redirection(""),
    cgi_extension(""),
    cgi_path("") {}

Location::Location(const ServerConfig& config) :
    path(""),
    root(config.root), // <- ici on copie la racine du serveur
    index(config.index),
    autoindex(config.autoindex),
    upload_enable(false),
    upload_store(""),
    redirection(""),
    cgi_extension(""),
    cgi_path("") {}

// struct ServerConfig {
//     std::string host;
// 	std::string root;
//     std::vector<int> port;
//     std::vector<std::string> server_name;
//     std::map<int, std::string> error_pages;
//     size_t client_max_body_size;
//     std::vector<Location> routes;
//     std::string index;
//     bool autoindex;

ServerConfig::ServerConfig() :
        host("0.0.0.0"),
        root("./"),
        port(80),
        server_name({"localhost"}),
        client_max_body_size(1000000) {}
