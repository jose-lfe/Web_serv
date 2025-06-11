#include "SimpleRouter.hpp"
#include <string>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>

void printServerConfig(const std::vector<ServerConfig>& configs) // debug function to print server configurations
{
    for (size_t i = 0; i < configs.size(); ++i) {
        const ServerConfig& conf = configs[i];
        std::cout << "=== Server " << i << " ===\n";
        std::cout << "Host: " << conf.host << "\n";
        std::cout << "Root: " << conf.root << "\n";
        std::cout << "Index: " << conf.index << "\n";
        std::cout << "Autoindex: " << (conf.autoindex ? "on" : "off") << "\n";
        std::cout << "Ports: ";
        for (size_t j = 0; j < conf.port.size(); ++j)
            std::cout << conf.port[j] << (j + 1 < conf.port.size() ? ", " : "");
        std::cout << "\nServer names: ";
        for (size_t j = 0; j < conf.server_name.size(); ++j)
            std::cout << conf.server_name[j] << (j + 1 < conf.server_name.size() ? ", " : "");
        std::cout << "\nError pages: ";
        for (std::map<int, std::string>::const_iterator it = conf.error_pages.begin(); it != conf.error_pages.end(); ++it)
            std::cout << it->first << "=>" << it->second << " ";
        std::cout << "\nClient max body size: " << conf.client_max_body_size << "\n";
        std::cout << "Locations:\n";
        for (size_t k = 0; k < conf.routes.size(); ++k) {
            const Location& loc = conf.routes[k];
            std::cout << "  - Path: " << loc.path << "\n";
            std::cout << "    Root: " << loc.root << "\n";
            std::cout << "    Index: " << loc.index << "\n";
            std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
            std::cout << "    Upload enable: " << (loc.upload_enable ? "on" : "off") << "\n";
            std::cout << "    Upload store: " << loc.upload_store << "\n";
            std::cout << "    Redirection: " << loc.redirection << "\n";
            std::cout << "    CGI extension: " << loc.cgi_extension << "\n";
            std::cout << "    CGI path: " << loc.cgi_path << "\n";
            std::cout << "    Methods: ";
            for (size_t m = 0; m < loc.methods.size(); ++m)
                std::cout << loc.methods[m] << (m + 1 < loc.methods.size() ? ", " : "");
            std::cout << "\n";
        }
        std::cout << "====================\n";
    }
}

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

/*
return buildHttpResponse("303 See Other", "text/plain", "Deleted. Redirecting...") + "Location: /photos\r\n\r\n";
tente de renvoyer une réponse HTTP qui :

Utilise le code 303 See Other (redirection après une action POST/DELETE).
Ajoute un en-tête Location: /photos pour dire au navigateur de revenir à la galerie après suppression.
Mais :
Dans ta fonction buildHttpResponse, tu ajoutes déjà les headers principaux. Pour une vraie redirection, il 
faut que l’en-tête Location soit inclus dans la partie headers, pas après le body.
Il vaudrait mieux faire :

std::string buildRedirectResponse(const std::string& location) {
    std::string response = "HTTP/1.1 303 See Other\r\n";
    response += "Location: " + location + "\r\n";
    response += "Content-Length: 0\r\n";
    response += "Connection: close\r\n\r\n";
    return response;
}
// ...dans route()...
if (suppression OK )
{
    return buildRedirectResponse("/photos");
}*/
//curl -X DELETE http://localhost:8080/photos/nom.jpg

std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body) {
	std::string response;
	response += "HTTP/1.1 " + status + "\r\n";
	response += "Content-Type: " + contentType + "\r\n";
	response += "Content-Length: " + to_string(body.size()) + "\r\n";
	response += "Connection: close\r\n";
	response += "\r\n";
	response += body;
	return response;
}

std::string extractQueryString(const std::string& url)
{
    std::string::size_type pos = url.find('?');
    if (pos == std::string::npos) {
        // Pas de query string
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
    return ""; // Valeur vide si non trouvé
}

std::vector<std::string> buildCgiEnv(const HandleRequest& req, const std::string& scriptPath) {
    std::vector<std::string> env;

    env.push_back("REQUEST_METHOD=" + req.method);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("SCRIPT_NAME=" + req.path); // ok tres certainement / ou pas peut etre faire l'inverse de la ligne du bas
    env.push_back("QUERY_STRING=" + extractQueryString(req.path)); // à parser depuis l'URL si besoin
    env.push_back("CONTENT_TYPE=" + trim(getHeaderValue(req.headers, "Content-Type")));
    env.push_back("CONTENT_LENGTH=" + trim(getHeaderValue(req.headers, "Content-Length")));
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=webserv");
    env.push_back("SERVER_NAME=localhost");
    env.push_back("SERVER_PORT=8080");
    env.push_back("REMOTE_ADDR=127.0.0.1"); // à adapter si tu as l'IP du client
    env.push_back("REDIRECT_STATUS=200");

    // Ajoute aussi les headers HTTP sous forme HTTP_*
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it) {
        std::string key = it->first;
        std::string value = it->second;
        if (key == "Content-Type" || key == "Content-Length")
            continue;
        // Transforme les - en _ et majuscules
        for (size_t i = 0; i < key.size(); ++i) {
            if (key[i] == '-') key[i] = '_';
            else key[i] = toupper(key[i]);
        }
        env.push_back("HTTP_" + key + "=" + value);
    }

    return env;
}

bool endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() < suffix.length())
		return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string exec_cgi(const HandleRequest& req)
{
    std::string scriptPath = "/home/jose-lfe/projects/web/with_cgi/www" + req.path;
    std::string interpreter = "/usr/bin/php-cgi";
    std::string newbody;

    std::vector<std::string> env_vec = buildCgiEnv(req, scriptPath);
    std::vector<char*> envp;
    for (size_t i = 0; i < env_vec.size(); ++i)
        envp.push_back(const_cast<char*>(env_vec[i].c_str()));
    envp.push_back(NULL);

    char* argv[3];
    argv[0] = const_cast<char*>(interpreter.c_str());
    argv[1] = const_cast<char*>(scriptPath.c_str());
    argv[2] = NULL;

    int in_pipe[2], out_pipe[2];
    pipe(in_pipe);
    pipe(out_pipe);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return buildHttpResponse("500 Internal Server Error", "text/plain", "Fork error");
    }

    if (pid == 0) {
        // Enfant
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        execve(interpreter.c_str(), argv, envp.data());
        perror("execve");
        exit(1);
    }

    // Parent
    close(in_pipe[0]);
    close(out_pipe[1]);

    // Envoie le body POST au CGI
    if (!req.body.empty()) {
        ssize_t written = write(in_pipe[1], req.body.c_str(), req.body.size());
        if (written < 0) perror("write to CGI stdin");
    }
    std::cerr << "Body size: " << req.body.size() << " / Content-Length: " << getHeaderValue(req.headers, "Content-Length") << std::endl; // pour debug
    close(in_pipe[1]);

    // Lis la sortie du CGI
    char buffer[4096];
    ssize_t n;
    while ((n = read(out_pipe[0], buffer, sizeof(buffer))) > 0) {
        newbody.append(buffer, n);
    }
    close(out_pipe[0]);
    int status;
    waitpid(pid, &status, 0);

    // Parsing CGI output
    std::string headers, body;
    size_t pos = newbody.find("\r\n\r\n");
    if (pos != std::string::npos) {
        headers = newbody.substr(0, pos);
        body = newbody.substr(pos + 4);
    } else {
        body = newbody;
    }

    std::string contentType = "text/html";
    std::istringstream hstream(headers);
    std::string line;
    while (std::getline(hstream, line)) {
        if (line.find("Content-Type:") == 0) {
            contentType = line.substr(13);
            while (!contentType.empty() && (contentType[0] == ' ' || contentType[0] == '\t'))
                contentType.erase(0, 1);
            if (!contentType.empty() && contentType.back() == '\r')
                contentType.pop_back();
            break;
        }
    }
    return buildHttpResponse("200 OK", contentType, body);
}

std::string generatePhotoEntriesHtml(const std::string& photosDir) {
	DIR* dir = opendir(photosDir.c_str());
	if (!dir) return "<li>Failed to open directory</li>";

	struct dirent* entry;
	std::ostringstream html;

	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		if (name == "." || name == "..") continue;

		std::string fullPath = photosDir + "/" + name;
		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
			html << "<li><img src =\"/photos/" << name
			<< "\" style=\"max-height:150px;\"><br>"
			<< name
			<< "<form method=\"POST\" action=\"/photos/" << name
			<< "?_method=DELETE\"><button type=\"submit\">Delete</button></form></li>";
		}
	}

	closedir(dir);
	return html.str();
}

const ServerConfig* findMatchingConfig(const HandleRequest& req, const std::vector<ServerConfig>& configs) {
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Host");
    if (it == req.headers.end())
        return NULL;

    for (size_t i = 0; i < configs.size(); ++i)
    {
        for (size_t j = 0; j < configs[i].server_name.size(); ++j)
        {
            if (it->second.find(configs[i].server_name[j]) != std::string::npos)
            {
                for (size_t k = 0; k < configs[i].port.size(); ++k)
                {
                    std::ostringstream portStr;
                    portStr << ":" << configs[i].port[k];
                    if (it->second.find(portStr.str()) != std::string::npos ||
                        (it->second.find(":") == std::string::npos && configs[i].port[k] == 80))
                    {
                        return &configs[i];
                    }
                }
            }
        }
    }
    return NULL;
}

const Location* findMatchingLocation(const HandleRequest& req, const std::vector<ServerConfig>& _configs, const ServerConfig** conf)
{
    const Location* bestLoc = NULL;
    const ServerConfig* tmp_conf = NULL;
    size_t bestLen = 0;
    for (int i = 0; i < _configs.size(); i++)
    {
        tmp_conf = &_configs[i];
        for (size_t i = 0; i < tmp_conf->routes.size(); ++i)
        {
            const Location& loc = tmp_conf->routes[i];
            if (req.path.find(loc.path) == 0 && loc.path.length() > bestLen)
            {
                bestLoc = &loc;
                bestLen = loc.path.length();
                *conf = tmp_conf;
            }
        }
    }

    // Si aucune location ne matche, cherche la location "/"
    if (!bestLoc) 
    {
        for (int i = 0; i < _configs.size(); i++)
        {
            tmp_conf = &_configs[i];
            for (size_t i = 0; i < tmp_conf->routes.size(); ++i)
            {
                if (tmp_conf->routes[i].path == "/") 
                {
                    *conf = tmp_conf;
                    return &tmp_conf->routes[i];
                }
            }
        }
    }

    if (!bestLoc)
        return NULL;
    
    return bestLoc;
}

bool isMethodAllowed(const Location* loc, const std::vector<std::string>& methods, const std::string& method)
{
    if (methods.empty())
    {
        return true;
    }
    for (int i = 0; i < loc->methods.size(); ++i)
    {
        if (loc->methods[i] == method)
        {
            return true;
        }
    }
    return false;
}

std::string getMimeType(const std::string& path) {
    std::map<std::string, std::string> mimeTypes;

    // Initialisation des types MIME connus
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".ico"] = "image/x-icon";
    mimeTypes[".svg"] = "image/svg+xml";

    // Trouver la dernière extension
    std::size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream"; // Par défaut si pas d'extension

    std::string ext = path.substr(dotPos);
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
    if (it != mimeTypes.end())
        return it->second;

    return "application/octet-stream"; // Type par défaut
}

std::string HandleGET(const HandleRequest& req, const std::vector<ServerConfig>& _configs)
{
//     const ServerConfig* conf = findMatchingConfig(req, _configs);
//     if (!conf) {
//        return buildHttpResponse("500 Internal Server Error", "text/plain", "No matching server configuration");
//     }
    const ServerConfig* conf;
    const Location* loc = findMatchingLocation(req, _configs, &conf);
    if (!loc) {
        return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
    }
    // Vérifie la redirection ici
    if (!loc->redirection.empty()) {
        // 302 Found ou 301 Moved Permanently selon ton besoin
        std::string response = "HTTP/1.1 302 Found\r\n";
        response += "Location: " + loc->redirection + "\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: close\r\n\r\n";
        return response;
    }
    if (!isMethodAllowed(loc, loc->methods, "GET")) {
        return buildHttpResponse("405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
    }
    // 1. Déterminer le root à utiliser
    std::string baseRoot;
    baseRoot = loc->root;
    // 2. Construire le chemin relatif à partir du path de la requête et du chemin de la location
    std::string relPath = req.path;
    if (relPath.find(loc->path) == 0)
        relPath = relPath.substr(loc->path.length());
    if (!relPath.empty() && relPath[0] == '/')
        relPath = relPath.substr(1);
    std::cout << std::endl << relPath << std::endl << std::endl;

    // 3. Construire le chemin réel
    std::string filePath = baseRoot;
    if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
        filePath += "/";
    filePath += relPath;
    std::cout << "GET filePath: " << filePath << std::endl; // pour debug

    // 4. Lire le fichier
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0) {
        return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
    }
    if (S_ISREG(st.st_mode))
    {
        std::cout << "Is a file: " << filePath << std::endl; // pour debug

        // Vérifie les droits d'accès en lecture
        if (access(filePath.c_str(), R_OK) != 0) {
            return buildHttpResponse("403 Forbidden", "text/plain", "Permission denied");
        }

        // Détermine le type MIME
        std::string contentType = getMimeType(filePath);

        // Lit le fichier
        std::ifstream file(filePath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to open file");
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string body = ss.str();

        return buildHttpResponse("200 OK", contentType, body);
    }
    else if (S_ISDIR(st.st_mode))
    {
        std::cout << "Is a directory: " << filePath << std::endl; //pour debug
        if (!loc->index.empty())
        {
            // Si c'est un répertoire, on cherche le fichier index
            std::string indexPath = filePath;
            if (indexPath.back() != '/')
                indexPath += "/";
            indexPath += loc->index;
            std::cout << "Index path: " << indexPath << std::endl; //pour debug


            std::ifstream f(indexPath.c_str());
            if (!f) {
                return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
            }
            std::ostringstream s;
            s << f.rdbuf();
            std::string body = s.str();
            return buildHttpResponse("200 OK", getMimeType(indexPath), body);
        }
        else if (loc->autoindex)
        {
            DIR* dir = opendir(filePath.c_str());
            if (!dir) {
                return buildHttpResponse("403 Forbidden", "text/plain", "Cannot open directory");
            }
            std::ostringstream html;
            html << "<html><head><title>Index of " << req.path << "</title></head><body>";
            html << "<h1>Index of " << req.path << "</h1><ul>";

            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                std::string name = entry->d_name;
                if (name == ".") continue;
                html << "<li><a href=\"" << req.path;
                if (req.path[req.path.size()-1] != '/')
                    html << "/";
                html << name << "\">" << name << "</a></li>";
            }
            closedir(dir);

            html << "</ul></body></html>";
            return buildHttpResponse("200 OK", "text/html", html.str());
        }
    }
    return buildHttpResponse("403 Forbidden", "text/plain", "403 Forbidden");
}

std::string SimpleRouter::route(const HandleRequest& req, const std::vector<ServerConfig>& _configs) {
	std::string method = req.method;
	std::string path = req.path;

    printServerConfig(_configs); // pour debug

    if (req.http_version != "HTTP/1.1")
        return buildHttpResponse("505 Bad HTTP version", "text/plain", "Bad HTTP version.");
	// Route pour la page d'accueil (album)
    if (method == "GET")
        return HandleGET(req, _configs);
	if (method == "GET" && (path == "/" || path == "/photos")) {
		std::string templateHtml = loadFile("www/template/gallery.html");
		if (templateHtml.empty()) {
			return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to load template.");
		}
		std::string photosHtml = generatePhotoEntriesHtml("www/photos");
		size_t pos = templateHtml.find("{{PHOTO_LIST}}");
		if (pos != std::string::npos) {
			templateHtml.replace(pos, 14, photosHtml);
			return buildHttpResponse("200 OK", "text/html", templateHtml);
		}
		return buildHttpResponse("500 Internal Server Error", "text/plain", "Template placeholder not found.");
	}

	// Nouvelle route : servir les images (jpg, png, etc.)
	if (method == "GET" && path.find("/photos/") == 0) {
		std::string filePath = "www" + path;  // exemple : /photos/photo1.jpg → www/photos/photo1.jpg
		std::ifstream file(filePath.c_str(), std::ios::binary);
		if (!file.is_open()) {
			return buildHttpResponse("404 Not Found", "text/plain", "Image not found");
		}

		std::ostringstream ss;
		ss << file.rdbuf();
		std::string body = ss.str();

		std::string contentType = "application/octet-stream";
		if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
			contentType = "image/jpeg";
		else if (path.find(".png") != std::string::npos)
			contentType = "image/png";
		else if (path.find(".gif") != std::string::npos)
			contentType = "image/gif";

		return buildHttpResponse("200 OK", contentType, body);
	}

    if (method == "POST" && path.find("/photos/") == 0 && extractQueryString(path).find("_method=DELETE") != std::string::npos)
    {
        std::string filePath = "www" + path.substr(0, path.find('?')); // retire la query string
        if (remove(filePath.c_str()) == 0)
        {
            return buildHttpResponse("303 See Other", "text/plain", "Deleted. Redirecting...")
            + "Location: /photos\r\n\r\n";
        }
        else
        {
        return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to delete file.");
        }
    }

	if (method == "POST" && endsWith(path, ".php")) // surement changer la logique en fonction des location mais pour l'instant c'est pour test
	{
		return exec_cgi(req);
		//return buildHttpResponse("200 OK", "text/plain", "PHP script executed successfully.");
	}

	// JSON endpoint (démo)
	if (method == "GET" && path == "/json") {
		return buildHttpResponse("200 OK", "application/json", "{\"msg\": \"Hello from JSON!\"}");
	}

	// Test POST
	if (method == "POST" && path == "/echo") {
		return buildHttpResponse("200 OK", "text/plain", "body");
	}

	// Fallback pour toutes les autres routes
	return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
}


// std::string SimpleRouter::route(const std::string& method, const std::string& path) {
// 		if (method == "GET" && (path == "/" || path.find("/photos") >= 0)) {
// 			std::string templateHtml = loadFile("www/template/gallery.html");
// 			if (templateHtml.empty()) {
// 				return buildHttpResponse("500 Internal Server Error", "text/plain", "Failed to load template.");
// 			}
// 			std::string photosHtml = generatePhotoEntriesHtml("www/photos");
// 			size_t pos = templateHtml.find("{{PHOTO_LIST}}");
// 			if (pos != std::string::npos) {
// 				templateHtml.replace(pos, 14, photosHtml);
// 				return buildHttpResponse("200 OK", "text/html", templateHtml);
// 			}
// 		}
// 		else if (method == "GET" && path == "/json") {
// 				return buildHttpResponse("200 OK", "application/json", "{\"msg\": \"Hello from JSON!\"}");
// 			}
// 		else if (method == "POST") {
// 			if (path == "/echo") {
// 				return buildHttpResponse("200 OK", "text/plain", "body");
// 			}
// 			else {
// 				return buildHttpResponse("404 Not Found", "text/plain", "404 Page Not Found");
// 			}
// 		}
// 		else {
// 			std::cout << path.find("/photos") << std::endl;
// 			return buildHttpResponse("405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
// 		}
// }