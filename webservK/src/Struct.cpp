#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <cstddef>
#include "Struct.hpp"




Location::Location() :
    path(""),
    root(""),
    index(""),
    client_max_body_size(0),
    autoindex(false),
    upload_enable(false),
    upload_store(""),
    redirection(""),
    cgi_extension(""),
    cgi_path("") {}

Location::Location(const ServerConfig& config) :
    path("/"),
    root(config.root),
    index(config.index),
    client_max_body_size(config.client_max_body_size),
    autoindex(config.autoindex),
    upload_enable(false),
    upload_store(""),
    redirection(""),
    cgi_extension(""),
    cgi_path("") {}

ServerConfig::ServerConfig()
    : host("0.0.0.0"),
      root("www/"),
      port(80),
      client_max_body_size(0) // fin de la liste d'init
{
    server_name.push_back("localhost"); // corps du constructeur
}
