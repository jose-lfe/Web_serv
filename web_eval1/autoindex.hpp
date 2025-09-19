#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <cstring>


std::string loadTemplate(const std::string& templatePath);
std::string formatFileRow(const std::string& href, const std::string& name, const std::string& size, const std::string& modTime);
std::string generateAutoindexHtml(const std::string& dirPath, const std::string& urlPath);

#endif
