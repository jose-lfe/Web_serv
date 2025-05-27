#include "SimpleRouter.hpp"
#include <string>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>


std::string loadFile(const std::string& path) {
	std::ifstream f(path.c_str());
	if (!f) return "";
	std::ostringstream s;
	s << f.rdbuf();
	return s.str();
}

std::string generatePhotoEntriesHtml(const std::string& photosDir) {
	DIR* dir = opendir(photosDir.c_str());
	if (!dir) return "<li>Failed to open directory</li>";

	struct dirent* entry;
	std::ostringstream html;

	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;

		std::string fullPath = photosDir + "/" + name;
		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
			html << "<li><img src =\"/photos/" << name
			<< "\" style=\"max-height:150px;\"><br>"
			<< name
			<< "<form method=\"POST\" action=\"/photos/" << name
			<< "?_method=DELETE\"><button type=\"submit\">Delete</button></form></li>";
		}
	}

	closedir(dir);
	return html.str();
}

std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body) {
	std::string response;
	response += "HTTP/1.1 " + status + "\r\n";
	response += "Content-Type: " + contentType + "\r\n";
	response += "Content-Length: " + to_string(body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	return response;
}

std::string SimpleRouter::route(const std::string& method, const std::string& path) {
	// Route pour la page d'accueil (album)
	if (method == "GET" && (path == "/" || path == "/photos")) {
		std::string templateHtml = loadFile("www/template/gallery.html");
		if (templateHtml.empty()) {
			return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to load template.");
		}
		std::string photosHtml = generatePhotoEntriesHtml("www/photos");
		size_t pos = templateHtml.find("{{PHOTO_LIST}}");
		if (pos != std::string::npos) {
			templateHtml.replace(pos, 14, photosHtml);
			return buildHttpResponse("200 OK", "text/html", templateHtml);
		}
		return buildHttpResponse("500 Internal Server Error", "text/plain", "Template placeholder not found.");
	}

	// Nouvelle route : servir les images (jpg, png, etc.)
	if (method == "GET" && path.find("/photos/") == 0) {
		std::string filePath = "www" + path;  // exemple : /photos/photo1.jpg → www/photos/photo1.jpg
		std::ifstream file(filePath.c_str(), std::ios::binary);
		if (!file.is_open()) {
			return buildHttpResponse("404 Not Found", "text/plain", "Image not found");
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		std::string body = ss.str();

		std::string contentType = "application/octet-stream";
		if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
			contentType = "image/jpeg";
		else if (path.find(".png") != std::string::npos)
			contentType = "image/png";
		else if (path.find(".gif") != std::string::npos)
			contentType = "image/gif";

		return buildHttpResponse("200 OK", contentType, body);
	}

	// JSON endpoint (démo)
	if (method == "GET" && path == "/json") {
		return buildHttpResponse("200 OK", "application/json", "{\"msg\": \"Hello from JSON!\"}");
	}

	// Test POST
	if (method == "POST" && path == "/echo") {
		return buildHttpResponse("200 OK", "text/plain", "body");
	}

	// Fallback pour toutes les autres routes
	return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
}


// std::string SimpleRouter::route(const std::string& method, const std::string& path) {
// 		if (method == "GET" && (path == "/" || path.find("/photos") >= 0)) {
// 			std::string templateHtml = loadFile("www/template/gallery.html");
// 			if (templateHtml.empty()) {
// 				return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to load template.");
// 			}
// 			std::string photosHtml = generatePhotoEntriesHtml("www/photos");
// 			size_t pos = templateHtml.find("{{PHOTO_LIST}}");
// 			if (pos != std::string::npos) {
// 				templateHtml.replace(pos, 14, photosHtml);
// 				return buildHttpResponse("200 OK", "text/html", templateHtml);
// 			}
// 		}
// 		else if (method == "GET" && path == "/json") {
// 				return buildHttpResponse("200 OK", "application/json", "{\"msg\": \"Hello from JSON!\"}");
// 			}
// 		else if (method == "POST") {
// 			if (path == "/echo") {
// 				return buildHttpResponse("200 OK", "text/plain", "body");
// 			}
// 			else {
// 				return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
// 			}
// 		}
// 		else {
// 			std::cout << path.find("/photos") << std::endl;
// 			return buildHttpResponse("405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
// 		}
// }