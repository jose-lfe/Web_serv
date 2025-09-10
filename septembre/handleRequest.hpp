#ifndef handleREQUEST_HPP
#define handleREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

class handleRequest {
public:
	std::string method;
	std::string path;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;

	bool parse(const std::string& raw_request);
	void print() const;
	std::string generateResponse() const;
};

#endif
