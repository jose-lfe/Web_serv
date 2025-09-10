#include "handleRequest.hpp"
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

bool handleRequest::parse(const std::string& raw_request)
{
	std::istringstream stream(raw_request);
	std::string line;

	if (!std::getline(stream, line))
		return false;

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::istringstream request_line(line);
	if (!(request_line >> method >> path >> http_version))
		return false;

	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty())
			break;

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		size_t end = value.size();
		while (end > 0 && std::isspace(static_cast<unsigned char>(value[end - 1])))
			--end;
		value.erase(end);

		headers[key] = value;
		
	}

	std::ostringstream body_stream;
	body_stream << stream.rdbuf();
	body = body_stream.str();
	
	return true;
}

void handleRequest::print() const {
	std::cout << "Method: " << method << "\n";
	std::cout << "Path: " << path << "\n";
	std::cout << "Version: " << http_version << "\n";
	std::cout << "Headers:\n";

	std::map<std::string, std::string>::const_iterator it;
	for (it = headers.begin(); it != headers.end(); ++it) {
		std::cout << " " << it->first << ":" << it->second << "\n";
	}

	if (!body.empty()) {
		std::cout << "Body:\n" << body << "\n";
	}
}
