#include <sys/stat.h>

const RouteConfig* findRouteConfig(const ServerConfig& serverConfig, const std::string& requestPath)
{
	for (size_t i = 0; i < serverConfig.routes.size(); ++i)
	{
		if (requestPath.find(serverConfig.routes[i].path) == 0)
			return &serverConfig.routes[i];
	}
	return NULL;
}


bool isValidRequest(const std::string& line, const ServerConfig& serverConfig)
{	
	std::string methode, path, version;
	std::istringstream iss(line);
	if (!(iss >> method >> path >> version))
		return false;
	
	const RouteConfig* route = findRouteConfig(serverConfig, path);
	if (!route)
		return false;

	if (std::find(route->methods.begin(), route->methods.end(), method) == route->methods.end())
		return false;

	if (version != "HTTP/1.1" && version != "HTTP/1.0")
		return false;

	return true;
}



void handleGetRequest(const std::string& path, int clientFd, const ServerConfig& serverConfig)
{
	const RouteConfig* route = findRouteConfig(serverConfig, path);
	if (!route)
	{
		sendErrorPage(clientFd, 404, serverConfig);
		return;
	}

	std::string fullPath = route->root + path.substr(route->path.size());
	std::ifstream file(fullPath);

	if (!file.is_open())
	{
		sendErrorPage(clientFd, 404, serverConfig);
		return;
	}

	std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::string response ="HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + std::to_string(fileContent.length()) + "\r\n\r\n";
	response += fileContent;

	send(clientFd, response.c_str(), response.lentgh(), 0);
}

void sendErrorPage(int clientFd, int errorCode, const ServerConfig& config)
{
	std::map<int, std::string>::const_iterator it = config.error_pages.find(errorCode);
	std::string body;
	if (it != config.error_pages.end() && fileExists(it->second))
	{
		std::ifstream file(it->second);
		body.assign((std::istreambuf:iterator<char>(file)), std::istream_iterator<char>());
	}
	else
	{
		body = std::to_string(errorCode) + "Error";
	}

	std::string response = "HTTP/1.1 " + std::to_string(errorCode) + "\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: " + std::to:string(body.length()) + "\r\n\r\n";
	response += body;

	send(clientFd, response.c_str(), response.length(), 0);
}

void handlePostRequest(const std::string& body, int clientFd)
{

	const RouteConfig* route = findRouteConfig(serverConfig, path);
	if (!route || std::find(route->methods.begin(), route->methods.end(), "POST") == route->methods.end())
	{
		sendErrorPage(clientFd, 405, serverConfig);
		return;
	}

	if (!route->upload_enable || route->upload_store.empty())
	{
		sendErrorPage(clientFd, 403, serverConfig);
		return;
	}

	std::string filename = route->upload_store + "/upload_" + std::to_string(std::time(NULL)) + ":txt";
	std::ofstream outFile(filename.c_str());
	if (!outFile.is_open())
	{
		sendErrorPage(clientFd, 500, serverConfig);
		return;
	}
	outFile << body;
	outFile.close();

	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/plain\r\n\r\n";
	response + "File uploaded successfully\n";
	send(clientFd, response.c_str(), response.length(), 0);
}

bool fileExists(const std::string& path)
{
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

void handleDeleteRequest(const std::string& path, int clientFd, const ServerConfig& serverConfig)
{
	const RouteConfig* route = findRouteConfig(serverConfig, path);
	if (!route || std::find(route->methods.begin(), route->methods.end(), "DELETE") == route->methods.end())
	{
		sendErrorPage(clientFd, 405, serverConfig);
		return;
	}

	std::string fullPath = route->root + path.substr(route->path.size());
	if (!fileExists(fullPath))
	{
		sendErrorPage(clientFd, 404, serverConfig);
		return;
	}

	if (remove(fullPath.c_str()) != 0)
	{
		sendErrorPage(clientFd, 500, serverConfig);
		return;
	}
	std::string response = "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/plain\r\n\r\n";
	response += "File deleted successfully.";
	send(clientFd, response.c_str(), response.length(), 0);
}