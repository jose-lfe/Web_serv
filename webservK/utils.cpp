#include "utils.hpp"

std::string loadFile(const std::string& path) {
	std::ifstream f(path.c_str());
	if (!f) return "";
	std::ostringstream s;
	s << f.rdbuf();
	return s.str();
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

std::string extractQueryString(const std::string& url)
{
    std::string::size_type pos = url.find('?');
    if (pos == std::string::npos) {
        return "";
    }
    return url.substr(pos + 1);
}

std::string getHeaderValue(const std::map<std::string, std::string>& headers, const std::string& key)
{
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}
bool isMethodAllowed(const Location* loc, const std::vector<std::string>& methods, const std::string& method)
{
    if (methods.empty())
    {
        return true;
    }
    for (size_t i = 0; i < loc->methods.size(); ++i)
    {
        if (loc->methods[i] == method)
        {
            return true;
        }
    }
    return false;
}
