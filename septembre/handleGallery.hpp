#ifndef HANDLEGALLERY_HPP
#define HANDLEGALLERY_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <dirent.h>
#include <sys/types.h>

std::string generatePhotoList(const std::string &dirPath);
std::string renderGallery(const std::string &templatePath, const std::string &photoDir);

#endif