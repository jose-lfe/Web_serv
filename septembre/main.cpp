#include "HttpServer.hpp"
std::vector<ServerConfig> parseConfig(const std::string& path, char **envp);


int main(int argc, char **argv, char **envp)
{
	std::vector<ServerConfig> configs;
	try
	{
		if (argc == 2)
		{
			configs = parseConfig(argv[1], envp);
		}
		else
			configs = parseConfig("config/confwithcgi.conf", envp);
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
