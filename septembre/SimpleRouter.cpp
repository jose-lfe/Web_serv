#include "SimpleRouter.hpp"
#include <string>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include "utils.hpp"


void printServerConfig(const std::vector<ServerConfig>& configs) // debug function to print server configurations
{
    for (size_t i = 0; i < configs.size(); ++i)
    {
        const ServerConfig& conf = configs[i];
        std::cout << "=== Server " << i << " ===\n";
        std::cout << "Host: " << conf.host << "\n";
        std::cout << "Root: " << conf.root << "\n";
        std::cout << "Index: " << conf.index << "\n";
        std::cout << "Autoindex: " << (conf.autoindex ? "on" : "off") << "\n";
        std::cout << "Ports: ";
        for (size_t j = 0; j < conf.port.size(); ++j)
            std::cout << conf.port[j] << (j + 1 < conf.port.size() ? ", " : "");
        std::cout << "\nServer names: ";
        for (size_t j = 0; j < conf.server_name.size(); ++j)
            std::cout << conf.server_name[j] << (j + 1 < conf.server_name.size() ? ", " : "");
        std::cout << "\nError pages: ";
        for (std::map<int, std::string>::const_iterator it = conf.error_pages.begin(); it != conf.error_pages.end(); ++it)
            std::cout << it->first << "=>" << it->second << " ";
        std::cout << "\nClient max body size: " << conf.client_max_body_size << "\n";
        std::cout << "Locations:\n";
        for (size_t k = 0; k < conf.routes.size(); ++k)
        {
            const Location& loc = conf.routes[k];
            std::cout << "  - Path: " << loc.path << "\n";
            std::cout << "    Root: " << loc.root << "\n";
            std::cout << "    Index: " << loc.index << "\n";
            std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
            std::cout << "    Upload enable: " << (loc.upload_enable ? "on" : "off") << "\n";
            std::cout << "    Upload store: " << loc.upload_store << "\n";
            std::cout << "    Redirection: " << loc.redirection << "\n";
            std::cout << "    CGI extension: " << loc.cgi_extension << "\n";
            std::cout << "    CGI path: " << loc.cgi_path << "\n";
            std::cout << "    Methods: ";
            for (size_t m = 0; m < loc.methods.size(); ++m)
            {
                std::cout << loc.methods[m] << (m + 1 < loc.methods.size() ? ", " : "");
                std::cout << "\n";
            }
            std::cout << "====================\n";
        }
    }
}
    

std::string SimpleRouter::route(const handleRequest& req, const std::vector<ServerConfig>& _configs)
{
	std::string method = req.method;
	std::string path = req.path;

    //printServerConfig(_configs); // pour debug
    if (req.http_version != "HTTP/1.1")
            return buildErrorResponse(505, _configs[0].error_pages);
    if (method == "GET")
        return handleGET(req, _configs);
    if (method == "POST" && extractQueryString(path).find("_method=DELETE") != std::string::npos)
        return handleDelete(req, _configs);
    if (method == "DELETE")
        return handleDelete(req, _configs);
    if (method == "POST")
        return handlePOST(req, _configs);
    return buildErrorResponse(501, _configs[0].error_pages);
}
