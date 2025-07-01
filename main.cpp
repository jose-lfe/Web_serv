#include "HttpServer.hpp"
std::vector<ServerConfig> parseConfig(const std::string& path);


int main(int argc, char **argv)
{
    std::vector<ServerConfig> configs;
    HttpServer* server = NULL;  // déclaration avant

    try
    {
        if (argc == 2)
            configs = parseConfig(argv[1]);
        else
            configs = parseConfig("config/confwithcgi.conf");

        server = new HttpServer(configs);  // instanciation

        if (!server->setupSockets())
        {
            std::cerr << "Failed to set up sockets\n";
            server->deleteSockets();
            delete server;
            return 1;
        }

        server->run();
        server->deleteSockets();
        delete server;
    }
    catch (std::exception &e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        if (server)
        {
            server->deleteSockets();
            delete server;
        }
        return 1;
    }

    return 0;
}
