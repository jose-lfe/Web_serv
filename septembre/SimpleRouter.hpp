#ifndef SIMPLEROUTER_HPP
#define SIMPLEROUTER_HPP

#include <string>
#include <map>
#include "utils.hpp"
#include "handleRequest.hpp"
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>
#include "Struct.hpp"
#include "autoindex.hpp"
#include "buildResponse.hpp"
#include "handleMethod.hpp"
#include "CGI.hpp"
#include "matchConfig.hpp"

class SimpleRouter {
public:
	static std::string route(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port);
	//static std::string route(const std::string& method, const std::string& path);
};

#endif
