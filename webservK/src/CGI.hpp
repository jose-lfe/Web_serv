#ifndef CGI_HPP
#define CGI_HPP

#include "SimpleRouter.hpp"

std::vector<std::string> buildCgiEnv(const handleRequest& req, const std::string& scriptPath, const Location *loc);
std::string exec_cgi(const handleRequest& req, const ServerConfig& configs, const Location *loc, std::string relPath);
bool endsWith(const std::string& str, const std::string& suffix);

#endif
