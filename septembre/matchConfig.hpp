#ifndef MATCHCONFIG_HPP
#define MATCHCONFIG_HPP

#include "HandleRequest.hpp"
#include "Struct.hpp"

const Location* findMatchingLocationWithName(const HandleRequest &req, const ServerConfig* conf);
const Location* findMatchingLocation(const HandleRequest& req, const std::vector<ServerConfig>& _configs, const ServerConfig** conf);

#endif
