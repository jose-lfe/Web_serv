#ifndef MATCHCONFIG_HPP
#define MATCHCONFIG_HPP

#include "handleRequest.hpp"
#include "Struct.hpp"

const Location* findMatchingLocationWithName(const handleRequest &req, const ServerConfig* conf);
const Location* findMatchingLocation(const handleRequest& req, const std::vector<ServerConfig>& _configs, const ServerConfig** conf, int port);

#endif
