#ifndef SIMPLEROUTER_HPP
#define SIMPLEROUTER_HPP

#include <string>
#include <map>
#include "utils.hpp"

class SimpleRouter {
public:
	static std::string route(const std::string& method, const std::string& path);
};

#endif