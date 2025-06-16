#ifndef SIMPLEROUTER_HPP
#define SIMPLEROUTER_HPP

#include <string>
#include <map>
#include "utils.hpp"
#include "HandleRequest.hpp"
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>
#include "Struct.hpp"
#include "autoindex.hpp"

class SimpleRouter {
public:
	static std::string route(const HandleRequest& req, const std::vector<ServerConfig>& _configs);
	//static std::string route(const std::string& method, const std::string& path);
};

#endif