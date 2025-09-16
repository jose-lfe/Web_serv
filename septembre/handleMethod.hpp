#ifndef handleMETHOD_HPP
#define handleMETHOD_HPP

#include "SimpleRouter.hpp"
#include "Struct.hpp"
std::string handleDelete(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port);
std::string handleGET(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port);
std::string handlePOST(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port);



#endif
