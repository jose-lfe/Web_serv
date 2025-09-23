#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include "SimpleRouter.hpp"
#include <vector>
#include "Struct.hpp"

template <typename T>
std::string to_string(const T& value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string loadFile(const std::string& path);
std::string trim(const std::string& s);
std::string extractQueryString(const std::string& url);
std::string getHeaderValue(const std::map<std::string, std::string>& headers, const std::string& key);
bool isMethodAllowed(const Location* loc, const std::vector<std::string>& methods, const std::string& method);

#endif
