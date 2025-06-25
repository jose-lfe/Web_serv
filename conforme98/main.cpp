#include "HttpServer.hpp"
std::vector<ServerConfig> parseConfig(const std::string& path);


int main(int argc, char **argv)
{
	std::vector<ServerConfig> configs;
	try
	{
		if (argc == 2)
		{
			configs = parseConfig(argv[1]);
		}
		else
			configs = parseConfig("config/confwithcgi.conf");
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