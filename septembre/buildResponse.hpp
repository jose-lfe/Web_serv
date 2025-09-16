#ifndef BUILDRESPONSE_HPP
#define BUILDRESPONSE_HPP

#include <string>
#include "handleRequest.hpp"
#include "SimpleRouter.hpp"

std::string buildErrorResponse(int error, std::map<int, std::string> error_pages);
std::string buildHtmlEchoResponse(const handleRequest& req);
std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body, bool keepAlive);
std::string buildRedirectionResponse(const std::string& location, const std::string& userAgent = "", const std::string& status = "303 See Other");
std::string getMimeType(const std::string& path);
#endif
