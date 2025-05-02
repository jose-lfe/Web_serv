#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

void trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(" \t\r\n"));
    str.erase(str.find_last_not_of(" \t\r\n") + 1);
}

void Config::parse(const std::string& filepath)
{
    std::ifstream file(filepath.c_str());
    if (!file.is_open())
	{
        std::cerr << "Cannot open config file: " << filepath << std::endl;
        return;
    }

    std::string line;
    bool in_server = false;
    bool in_location = false;
    LocationConfig current_location;

    while (std::getline(file, line))
	{
        trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "server {")
		{
            in_server = true;
        }
		else if (line == "}")
		{
            if (in_location)
			{
                server.locations.push_back(current_location);
                in_location = false;
            }
			else
			{
                in_server = false;
            }
        }
		else if (in_server && line.find("listen") == 0)
		{
            std::istringstream iss(line);
            std::string key;
            int port;
            iss >> key >> port;
            server.listen_port = port;
        }
		else if (in_server && line.find("server_name") == 0)
		{
            std::istringstream iss(line);
            std::string key, name;
            iss >> key >> name;
            server.server_name = name;
        }
		else if (in_server && line.find("location") == 0)
		{
            std::istringstream iss(line);
            std::string key, path, brace;
            iss >> key >> path >> brace;
            current_location = LocationConfig();
            current_location.path = path;
            in_location = true;
        }
		else if (in_location && line.find("root") == 0)
		{
            std::istringstream iss(line);
            std::string key, root_path;
            iss >> key >> root_path;
            current_location.root = root_path;
        }
    }
    file.close();
}
