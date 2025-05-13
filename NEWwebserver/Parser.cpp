#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include "Struct.hpp"

bool stringToBool(const std::string& s) {
    return s == "on" || s == "true" || s == "1";
}

int stringToInt(const std::string& s) {
    return std::atoi(s.c_str());
}

size_t stringToSize(const std::string& s) {
    return static_cast<size_t>(std::strtol(s.c_str(), NULL, 10));
}

std::vector<std::string> splitWords(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word)
        words.push_back(word);
    return words;
}

std::string removeSemicolon(const std::string& token) {
    if (!token.empty() && token[token.size() - 1] == ';')
        return token.substr(0, token.size() - 1);
    return token;
}

std::vector<ServerConfig> parseConfig(const std::string& path) {
    std::ifstream infile(path.c_str());
    std::string line;
    std::vector<ServerConfig> servers;
    ServerConfig current;
    bool in_server = false;
    bool in_location = false;
    Location current_location;

    while (std::getline(infile, line)) {
        std::vector<std::string> tokens = splitWords(line);
        if (tokens.empty())
            continue;

        if (tokens[0] == "server" && tokens.size() == 2 && tokens[1] == "{") {
            current = ServerConfig();
            in_server = true;
        }
        else if (tokens[0] == "location" && tokens.size() >= 2) {
            in_location = true;
            current_location = Location();
            current_location.path = tokens[1];
        }
        else if (tokens[0] == "}") {
            if (in_location) {
                current.routes.push_back(current_location);
                in_location = false;
            } else if (in_server) {
                servers.push_back(current);
                in_server = false;
            }
        }
        else if (in_server || in_location) {
            std::string key = tokens[0];
            if (tokens.size() >= 2) {
                std::string value = removeSemicolon(tokens[1]);

                // traitons les directives
                if (key == "listen") {
                    size_t pos = value.find(":");
                    if (pos != std::string::npos) {
                        current.host = value.substr(0, pos);
                        current.port.clear(); // écrase s'il y en avait
                        current.port.push_back(stringToInt(value.substr(pos + 1)));
                    } else {
                        current.port.clear();
                        current.port.push_back(stringToInt(value));
                    }
                } else if (key == "server_name") {
                    current.server_name.clear();
                    for (size_t i = 1; i < tokens.size(); ++i)
                        current.server_name.push_back(removeSemicolon(tokens[i]));
                } else if (key == "root") {
                    if (in_location)
                        current_location.root = value;
                    else
                        current.root = value;
                } else if (key == "index") {
                    if (in_location)
                        current_location.index = value;
                } else if (key == "autoindex") {
                    if (in_location)
                        current_location.autoindex = stringToBool(value);
                } else if (key == "client_max_body_size") {
                    current.client_max_body_size = stringToSize(value);
                }
				else if (key == "error_page" && tokens.size() >= 3) {
					int code = std::atoi(tokens[1].c_str());
					std::string page = removeSemicolon(tokens[2]);
					current.error_pages[code] = page;
				}
				else if (key == "upload_store") {
					if (in_location)
						current_location.upload_store = value;
				}
				else if (key == "upload_enable") {
					if (in_location)
						current_location.upload_enable = stringToBool(value);
				}
				else if (key == "redirection") {
					if (in_location)
						current_location.redirection = value;
				}
				else if (key == "cgi_path") {
					if (in_location)
						current_location.cgi_path = value;
				}
				else if (key == "cgi_extension") {
					if (in_location)
						current_location.cgi_extension = value;
				}
			}
		}
	}
    return servers;
}
