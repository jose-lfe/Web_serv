#ifndef HANDLEMETHOD_HPP
#define HANDLEMETHOD_HPP

#include "SimpleRouter.hpp"
#include "Struct.hpp"
std::string handleDelete(const HandleRequest& req, const std::vector<ServerConfig>& _configs);
std::string HandleGET(const HandleRequest& req, const std::vector<ServerConfig>& _configs);
std::string HandlePOST(const HandleRequest& req, const std::vector<ServerConfig>& _configs);



#endif
