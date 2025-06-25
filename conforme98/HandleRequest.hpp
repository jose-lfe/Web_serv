#ifndef HANDLEREQUEST_HPP
#define HANDLEREQUEST_HPP

#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <algorithm>

class HandleRequest {
public:
	std::string method;
	std::string path;
	std::string http_version;
	std::map<std::string, std::string> headers;
	std::string body;

bool parse(const std::string& raw_request) {
	std::istringstream stream(raw_request);
	std::string line;

	if (!std::getline(stream, line))
		return false;

	// Remplacer line.back() == '\r' par version C++98
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

		// Trim de droite (retirer les espaces en fin de chaîne)
		size_t end = value.size();
		while (end > 0 && std::isspace(value[end - 1]))
			--end;
		value.erase(end);

		headers[key] = value;
	}

	// Lire le reste comme body (façon manuelle C++98)
	std::ostringstream body_stream;
	body_stream << stream.rdbuf();
	body = body_stream.str();

	return true;
}

// 	bool parse(const std::string& raw_request) {
// 		std::istringstream stream(raw_request);
// 		std::string line;

// 		if (!std::getline(stream, line)) return false;
// 		if (line.back() == '\r') line.pop_back();

// 		std::istringstream request_line(line);
// 		if (!(request_line >> method >> path >> http_version)) return false;

// 		while (std::getline(stream, line)) {
// 			if (line.back() == '\r') line.pop_back();
// 			if (line.empty()) break;

// 			size_t colon = line.find(':');
// 			if (colon == std::string::npos) continue;

// 			std::string key = line.substr(0, colon);
// 			std::string value = line.substr(colon + 1);

// 			value.erase(std::find_if(value.rbegin(), value.rend(), [](int ch) {
// 				return !std::isspace(ch);
// 			}).base(), value.end());

// 			headers[key] = value;
// 	}

// 	std::string body_content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
// 	body = body_content;

// 	return true;
// }

void print() const {
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


std::string generateResponse() const {
	std::string status_line;
	std::string body;
	std::string content_type = "text/plain";

	if (method != "GET")
	{
		status_line = "HTTP/1.1 405 Method not Allowed\r\n";
		body = "405 Method Not Allowed";
	} else if (path == "/" || path == "index.html") {
		status_line = "HTTP/1.1 200 OK\r\n";
		body = "Hello there!";
	} else {
		status_line = "HTTP/1.1 404 Not Found\r\n";
		body = "404 Page Not Found";
	}

	std::ostringstream response;
	response << status_line;
	response << "Content-Type: " << content_type << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body;

	return response.str();
}
};

#endif