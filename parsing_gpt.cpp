#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib> // atoi, strtol
#include <cstddef> // size_t

struct RouteConfig {
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

    RouteConfig() :
        autoindex(false),
        upload_enable(false) {}
};

struct ServerConfig {
    std::string host;
    int port;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_pages;
    size_t client_max_body_size;
    std::vector<RouteConfig> routes;

    ServerConfig() :
        port(80),
        client_max_body_size(1000000) {}
};

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string word;
    while (iss >> word)
        tokens.push_back(word);
    return tokens;
}

void parseRoute(std::istream& in, RouteConfig& route, const std::string& path) {
    route.path = path;
    std::string line;
    while (std::getline(in, line)) {
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty())
            continue;
        if (tokens[0] == "}") break;

        if (tokens[0] == "methods") {
            for (size_t i = 1; i < tokens.size(); ++i)
                route.methods.push_back(tokens[i]);
        } else if (tokens[0] == "root") {
            if (tokens.size() > 1)
                route.root = tokens[1];
        } else if (tokens[0] == "index") {
            if (tokens.size() > 1)
                route.index = tokens[1];
        } else if (tokens[0] == "autoindex") {
            if (tokens.size() > 1)
                route.autoindex = (tokens[1] == "on");
        } else if (tokens[0] == "upload_enable") {
            if (tokens.size() > 1)
                route.upload_enable = (tokens[1] == "on");
        } else if (tokens[0] == "upload_store") {
            if (tokens.size() > 1)
                route.upload_store = tokens[1];
        } else if (tokens[0] == "return") {
            if (tokens.size() > 1)
                route.redirection = tokens[1];
        } else if (tokens[0] == "cgi_extension") {
            if (tokens.size() > 1)
                route.cgi_extension = tokens[1];
        } else if (tokens[0] == "cgi_path") {
            if (tokens.size() > 1)
                route.cgi_path = tokens[1];
        }
    }
}

void parseServer(std::istream& in, ServerConfig& server) {
    std::string line;
    while (std::getline(in, line)) {
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty())
            continue;
        if (tokens[0] == "}") break;

        if (tokens[0] == "listen" && tokens.size() > 1) {
            std::string hostport = tokens[1];
            size_t colon = hostport.find(':');
            if (colon != std::string::npos) {
                server.host = hostport.substr(0, colon);
                server.port = atoi(hostport.substr(colon + 1).c_str());
            } else {
                server.port = atoi(hostport.c_str());
            }
        } else if (tokens[0] == "server_name") {
            for (size_t i = 1; i < tokens.size(); ++i)
                server.server_names.push_back(tokens[i]);
        } else if (tokens[0] == "error_page" && tokens.size() >= 3) {
            int code = atoi(tokens[1].c_str());
            server.error_pages[code] = tokens[2];
        } else if (tokens[0] == "client_max_body_size" && tokens.size() > 1) {
            std::string sizeStr = tokens[1];
            size_t multiplier = 1;
            if (sizeStr[sizeStr.size() - 1] == 'M') {
                multiplier = 1024 * 1024;
                sizeStr = sizeStr.substr(0, sizeStr.size() - 1);
            }
            server.client_max_body_size = strtol(sizeStr.c_str(), NULL, 10) * multiplier;
        } else if (tokens[0] == "route" && tokens.size() > 1) {
            std::string path = tokens[1];
            std::getline(in, line); // skip {
            RouteConfig route;
            parseRoute(in, route, path);
            server.routes.push_back(route);
        }
    }
}

std::vector<ServerConfig> parseConfigFile(const std::string& filename) {
    std::ifstream file(filename.c_str());
    std::vector<ServerConfig> servers;

    if (!file) {
        std::cerr << "Erreur d'ouverture du fichier : " << filename << std::endl;
        return servers;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = tokenize(line);
        if (tokens.empty()) continue;

        if (tokens[0] == "server") {
            while (line.find('{') == std::string::npos && std::getline(file, line)) {
                // Skip lines until we find the opening brace
            }
            ServerConfig server;
            parseServer(file, server);
            servers.push_back(server);
        }
    }

    return servers;
}

// Exemple d'affichage
void printConfig(const std::vector<ServerConfig>& servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& s = servers[i];
        std::cout << "Server " << i << " on " << s.host << ":" << s.port << std::endl;
        std::cout << "  Server names: ";
        for (size_t j = 0; j < s.server_names.size(); ++j)
            std::cout << s.server_names[j] << " ";
        std::cout << "\n  Client max body size: " << s.client_max_body_size << " bytes" << std::endl;
        std::cout << "  Error pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator it = s.error_pages.begin(); it != s.error_pages.end(); ++it)
            std::cout << "    " << it->first << " => " << it->second << std::endl;

        for (size_t j = 0; j < s.routes.size(); ++j) {
            const RouteConfig& r = s.routes[j];
            std::cout << "  Route: " << r.path << std::endl;
            std::cout << "    Methods: ";
            for (size_t k = 0; k < r.methods.size(); ++k)
                std::cout << r.methods[k] << " ";
            std::cout << "\n    Root: " << r.root
                      << "\n    Index: " << r.index
                      << "\n    Autoindex: " << (r.autoindex ? "on" : "off")
                      << "\n    Upload enable: " << (r.upload_enable ? "on" : "off")
                      << "\n    Upload store: " << r.upload_store
                      << "\n    Redirection: " << r.redirection
                      << "\n    CGI extension: " << r.cgi_extension
                      << "\n    CGI path: " << r.cgi_path << std::endl;
        }
    }
}

int main() {
    std::vector<ServerConfig> servers = parseConfigFile("config/exemple3.conf");
    printConfig(servers);
    return 0;
}

//g++ -std=c++98 -Wall -Wextra -Werror -o parser_gpt parsing_gpt.cpp