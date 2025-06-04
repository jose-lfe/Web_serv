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

		if (!std::getline(stream, line)) return false;
		if (line.back() == '\r') line.pop_back();

		std::istringstream request_line(line);
		if (!(request_line >> method >> path >> http_version)) return false;

		while (std::getline(stream, line)) {
			if (line.back() == '\r') line.pop_back();
			if (line.empty()) break;

			size_t colon = line.find(':');
			if (colon == std::string::npos) continue;

			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);

			value.erase(std::find_if(value.rbegin(), value.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), value.end());

			headers[key] = value;
	}

	std::string body_content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	body = body_content;

	return true;
}

void print() const {
	std::cout << "Method: " << method << "\n";
	std::cout << "Path: " << path << "\n";
	std::cout << "Version: " << http_version << "\n";
	std::cout << "Headers:\n";
	for (const auto& h : headers) {
		std::cout << " " << h.first << ":" << h.second << "\n";
	}
	if (!body.empty())
	{
		std::cout << "Body:\n" << body << "\n";
	}
}
};