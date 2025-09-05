#include "buildResponse.hpp"

std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body)
{
    std::string response;
    response += "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "Server: webserv/1.0\r\n";
    response += "\r\n";
    response += body;
    return response;
}

std::string buildHtmlEchoResponse(const handleRequest& req)
{
    std::string response;

	std::string body =
		"<html>\n"
		"  <head><title>POST Echo</title></head>\n"
		"  <body>\n"
		"    <h1>POST data received:</h1>\n"
		"    <pre>" + req.body + "</pre>\n"
		"  </body>\n"
		"</html>";

    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "Server: webserv/1.0\r\n";
    response += "\r\n";
    response += body;

	return response;
}

std::string buildErrorResponse(int error, std::map<int, std::string> error_pages)
{
    std::string response;
    std::string filePath;
    std::string status;
    std::string body;

	std::cout << error << ": builderrorresponse" << std::endl; //debug
if (error_pages.find(error) != error_pages.end())
{
    filePath = error_pages.find(error)->second;
}
else
{
switch (error)
{
    case 400:
        filePath = "./error_pages/error_400.html";
        break;

    case 403:
        filePath = "./error_pages/error_403.html";
        break;
    case 404: 
        filePath = "./error_pages/error_404.html";
        break;
    case 405:
        filePath = "./error_pages/error_405.html";
        break;
    case 500:
        filePath = "./error_pages/error_500.html";
        break;
    case 501:
        filePath = "./error_pages/error_501.html";
        break;
    case 505:
        filePath = "./error_pages/error_505.html";
        break;
    default:
        filePath = "./errors/default.html";  // fallback file
}   
}

    switch (error) {
    case 400:
        status = "400 Bad Request";
        break;

    case 403:
        status = "403 Forbidden";
        break;
    case 404: 
        status = "404 Not Found";
        break;
    case 405:
        status = "405 Method Not Allowed";
        break;
    case 500:
        status = "500 Internal Server Error";
        break;
    case 501:
        status = "501 Not Implemented";
        break;
    case 505:
        status = "505 HTTP Version Not Supported";
        break;
    default:
        status = "Error";
    }
    

std::ifstream file(filePath.c_str(), std::ios::binary);
if (!file.is_open()) {
    std::cout << filePath << std::endl;
    return buildHttpResponse("404 Not Found", "text/html", "404 Page Not Found");
}
std::ostringstream ss;
ss << file.rdbuf();
body = ss.str();

response += "HTTP/1.1 " + status + "\r\n";
response += "Content-Type: text/html\r\n";
response += "Content-Length: " + to_string(body.size()) + "\r\n";
response += "Connection: close\r\n";
response += "Server: webserv/1.0\r\n";
response += "\r\n";
response += body;

return response;
}

std::string buildRedirectionResponse(const std::string& location,
									 const std::string& userAgent,
									 const std::string& status) {
	std::ostringstream response;
	std::string body;
	std::string contentType;

	// Détection simple de curl
	bool isCurl = userAgent.find("curl") != std::string::npos;

	if (isCurl) {
		contentType = "text/plain";
		body = "Redirecting to: " + location + "\n";
	} else {
		contentType = "text/html";
		std::ostringstream htmlBody;
		htmlBody << "<html><head><meta http-equiv=\"refresh\" content=\"1; URL=" << location << "\">"
				 << "<title>Redirecting...</title></head><body>"
				 << "<h1>Redirecting...</h1>"
				 << "<p>If you're not redirected automatically, <a href=\"" << location << "\">click here</a>.</p>"
				 << "</body></html>";
		body = htmlBody.str();
	}

	// Construction de la réponse complète
	response << "HTTP/1.1 " << status << "\r\n";
	response << "Location: " << location << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body;

	return response.str();
}

std::string getMimeType(const std::string& path) {
    std::map<std::string, std::string> mimeTypes;

    // Initialisation des types MIME connus
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".svg"] = "image/svg+xml";

    // Trouver la dernière extension
    std::size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream"; // Par défaut si pas d'extension

    std::string ext = path.substr(dotPos);
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
    if (it != mimeTypes.end())
        return it->second;

    return "application/octet-stream"; // Type par défaut
}
