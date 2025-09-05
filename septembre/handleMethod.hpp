#ifndef handleMETHOD_HPP
#define handleMETHOD_HPP

#include "SimpleRouter.hpp"
#include "Struct.hpp"
std::string handleDelete(const handleRequest& req, const std::vector<ServerConfig>& _configs);
std::string handleGET(const handleRequest& req, const std::vector<ServerConfig>& _configs);
std::string handlePOST(const handleRequest& req, const std::vector<ServerConfig>& _configs);



#endif
