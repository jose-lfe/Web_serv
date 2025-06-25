#include "autoindex.hpp"
#include "SimpleRouter.hpp"

std::string loadTemplate(const std::string& templatePath) {
    std::ifstream file(templatePath.c_str());
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string formatFileRow(const std::string& href, const std::string& name, const std::string& size, const std::string& modTime) {
    std::ostringstream row;
    row << "<tr><td><a href=\"" << href << "\">" << name << "</a></td>"
        << "<td>" << size << "</td><td>" << modTime << "</td></tr>\n";
    return row.str();
}

std::string generateAutoindexHtml(const std::string& dirPath, const std::string& urlPath) {
	std::string templateHtml = loadTemplate("./autoindex/autoindex.html");
	if (templateHtml.empty()) {
		return "500";
	}

	DIR* dir = opendir(dirPath.c_str());
	if (!dir) {
		return "403";
	}

	std::ostringstream rows;

	// Parent folder link
	if (urlPath != "/") {
		std::string parent = urlPath;
		if (!parent.empty() && parent[parent.size() - 1] == '/')
			parent.erase(parent.size() - 1);
		size_t slash = parent.find_last_of('/');
		parent = (slash != std::string::npos) ? parent.substr(0, slash) : "/";
		if (parent.empty()) parent = "/";
		rows << formatFileRow(parent + "/", "../", "", "");
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == ".") continue;

		std::string fullPath = dirPath + "/" + name;
		struct stat fileStat;
		if (stat(fullPath.c_str(), &fileStat) == -1) continue;

		std::string href = urlPath;
		if (href.empty() || href[href.size() - 1] != '/')
			href += "/";
		href += name;
		if (S_ISDIR(fileStat.st_mode))
			href += "/";

		// Convert file size to string (remplace std::to_string)
		std::ostringstream sizeStream;
		if (S_ISDIR(fileStat.st_mode)) {
			sizeStream << "Directory";
		} else {
			sizeStream << fileStat.st_size << " B";
		}
		std::string size = sizeStream.str();

		char timebuf[80];
		strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));

		rows << formatFileRow(href, name, size, timebuf);
	}
	closedir(dir);

	// Replace placeholders
	size_t pathPos = templateHtml.find("{{PATH}}");
	while (pathPos != std::string::npos) {
		templateHtml.replace(pathPos, 8, urlPath);
		pathPos = templateHtml.find("{{PATH}}");
	}
	size_t rowsPos = templateHtml.find("{{ROWS}}");
	if (rowsPos != std::string::npos)
		templateHtml.replace(rowsPos, 8, rows.str());

	return templateHtml;
}


// std::string generateAutoindexHtml(const std::string& dirPath, const std::string& urlPath) {
//     std::string templateHtml = loadTemplate("./autoindex/autoindex.html");
//     if (templateHtml.empty())
// 	{
// 		return "500";
// 	}

//     DIR* dir = opendir(dirPath.c_str());
//     if (!dir)
// 	{
// 		return "403";
// 	}
//     std::ostringstream rows;

//     // Parent folder link
//     if (urlPath != "/") {
//         std::string parent = urlPath;
//         if (parent.back() == '/') parent.pop_back();
//         size_t slash = parent.find_last_of('/');
//         parent = (slash != std::string::npos) ? parent.substr(0, slash) : "/";
//         if (parent.empty()) parent = "/";
//         rows << formatFileRow(parent + "/", "../", "", "");
//     }

//     struct dirent* entry;
//     while ((entry = readdir(dir)) != NULL) {
//         std::string name = entry->d_name;
//         if (name == ".") continue;

//         std::string fullPath = dirPath + "/" + name;
//         struct stat fileStat;
//         if (stat(fullPath.c_str(), &fileStat) == -1) continue;

//         std::string href = urlPath;
//         if (href.back() != '/') href += "/";
//         href += name;
//         if (S_ISDIR(fileStat.st_mode)) href += "/";

//         std::string size = S_ISDIR(fileStat.st_mode) ? "Directory" : std::to_string(fileStat.st_size) + " B";

//         char timebuf[80];
//         strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));

//         rows << formatFileRow(href, name, size, timebuf);
//     }
//     closedir(dir);

//     // Replace placeholders
//     size_t pathPos = templateHtml.find("{{PATH}}");
//     while (pathPos != std::string::npos) {
//         templateHtml.replace(pathPos, 8, urlPath);
//         pathPos = templateHtml.find("{{PATH}}");
//     }
//     size_t rowsPos = templateHtml.find("{{ROWS}}");
//     if (rowsPos != std::string::npos)
//         templateHtml.replace(rowsPos, 8, rows.str());

//     return templateHtml;
// }