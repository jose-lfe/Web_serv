#include "HttpServer.hpp"
std::vector<ServerConfig> parseConfig(const std::string& path);


int main(int argc, char **argv)
{
	try
	{
		std::vector<ServerConfig> configs = parseConfig("config/webserv.conf");
		HttpServer server(configs);

		if (!server.setupSockets())
		{
			std::cerr << "Failed to set up sockets\n";
			return 1;
		}

		server.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Fatal: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}