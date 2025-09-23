#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include "Struct.hpp"

std::vector<ServerConfig> parseConfig(const std::string& path, char **envp);

//utils
bool stringToBool(const std::string& s);
int stringToInt(const std::string& s);
size_t stringToSize(const std::string& s);
std::vector<std::string> splitWords(const std::string& line);
std::string removeSemicolon(const std::string& token);

#endif