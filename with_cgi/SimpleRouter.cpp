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

// std::string buildRedirectionResponse(const Location& loc)
// {
//     std::string response = "HTTP/1.1 303 See Other\r\n";
//     response += "Location: " + loc.redirection + "\r\n";
//     response += "Content-Length: 0\r\n";
//     response += "Connection: close\r\n\r\n";
//     return response;
// }
    
std::string buildHttpResponse(const std::string& status, const std::string& contentType, const std::string& body)
{
    std::string response;
    response += "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "Server: webserv/1.0\r\n";
    response += "\r\n";
    response += body;
    return response;
}

std::string buildHtmlEchoResponse(const HandleRequest& req)
{
    std::string response;

	std::string body =
		"<html>\n"
		"  <head><title>POST Echo</title></head>\n"
		"  <body>\n"
		"    <h1>POST data received:</h1>\n"
		"    <pre>" + req.body + "</pre>\n"
		"  </body>\n"
		"</html>";

    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html; charset=UTF-8\r\n";
    response += "Content-Length: " + to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "Server: webserv/1.0\r\n";
    response += "\r\n";
    response += body;

	return response;
}

std::string buildErrorResponse(int error, std::map<int, std::string> error_pages)
{
    std::string response;
    std::string filePath;
    std::string status;
    std::string body;

	std::cout << error << ": builderrorresponse" << std::endl; //debug
if (error_pages.find(error) != error_pages.end())
{
    filePath = error_pages.find(error)->second;
}
else
{
switch (error)
{
    case 400:
        filePath = "./error_pages/error_400.html";
        break;

    case 403:
        filePath = "./error_pages/error_403.html";
        break;
    case 404: 
        filePath = "./error_pages/error_404.html";
        break;
    case 405:
        filePath = "./error_pages/error_405.html";
        break;
    case 500:
        filePath = "./error_pages/error_500.html";
        break;
    case 505:
        filePath = "./error_pages/error_505.html";
        break;
    default:
        filePath = "./errors/default.html";  // fallback file
}   
}

    switch (error) {
    case 400:
        status = "400 Bad Request";
        break;

    case 403:
        status = "403 Forbidden";
        break;
    case 404: 
        status = "404 Not Found";
        break;
    case 405:
        status = "405 Method Not Allowed";
        break;
    case 500:
        status = "500 Internal Server Error";
        break;
    case 505:
        status = "505 HTTP Version Not Supported";
        break;
    default:
        status = "Error";
    }
    

std::ifstream file(filePath.c_str(), std::ios::binary);
if (!file.is_open()) {
    std::cout << filePath << std::endl;
    return buildHttpResponse("404 Not Found", "text/html", "404 Page Not Found");
}
std::ostringstream ss;
ss << file.rdbuf();
body = ss.str();

response += "HTTP/1.1 " + status + "\r\n";
response += "Content-Type: text/html\r\n";
response += "Content-Length: " + to_string(body.size()) + "\r\n";
response += "Connection: close\r\n";
response += "Server: webserv/1.0\r\n";
response += "\r\n";
response += body;

return response;
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

std::vector<std::string> buildCgiEnv(const HandleRequest& req, const std::string& scriptPath, const Location *loc) {
    std::vector<std::string> env;

    env.push_back("REQUEST_METHOD=" + req.method);
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("PATH_INFO=" + req.path);
    env.push_back("UPLOAD_DIR=" + loc->upload_store);
    env.push_back("SCRIPT_NAME=" + req.path); // ok tres certainement / ou pas peut etre faire l'inverse de la ligne du bas
    env.push_back("QUERY_STRING=" + extractQueryString(req.path)); // à parser depuis l'URL si besoin
    env.push_back("CONTENT_TYPE=" + trim(getHeaderValue(req.headers, "Content-Type")));
    env.push_back("CONTENT_LENGTH=" + to_string(req.body.size()));
    //env.push_back("CONTENT_LENGTH=" + trim(getHeaderValue(req.headers, "Content-Length")));
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

std::string exec_cgi(const HandleRequest& req, const ServerConfig& configs, const Location *loc, std::string relPath)
//std::string exec_cgi(const HandleRequest& req, const std::vector<ServerConfig>& configs)
{
    std::cout << "inside exec_cgi" << std::endl; // debug
    std::string scriptPath = "/home/jose-lfe/projects/web/with_cgi/www/" + relPath;
    std::string interpreter = loc->cgi_path;
    std::string newbody;
    std::cout << "script path: " << scriptPath << std::endl; // debug


    std::vector<std::string> env_vec = buildCgiEnv(req, scriptPath, loc);
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
        return buildErrorResponse(500, configs.error_pages);
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
    size_t total_written = 0;
    while (total_written < req.body.size()) {
        ssize_t written = write(in_pipe[1], req.body.c_str() + total_written, req.body.size() - total_written);
        if (written < 0) {
            perror("write to CGI stdin");
            break;
        }
        total_written += written;
    }
    std::cerr << "Total written to CGI stdin: " << total_written << " bytes" << std::endl;
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
    std::cerr << "newbody size: " << newbody.size() << " body size: " << body.size() << std::endl;
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

const Location* findMatchingLocationWithName(const HandleRequest &req, const ServerConfig* conf)
{
    std::cout << "trouver le serveur_name" << std::endl;
    const Location* bestLoc = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < conf->routes.size(); ++i)
    {
        const Location& loc = conf->routes[i];
        if (req.path.find(loc.path) == 0 && loc.path.length() > bestLen)
        {
            bestLoc = &loc;
            bestLen = loc.path.length();
        }
    }
    if (!bestLoc)
    {
        for (size_t i = 0; i < conf->routes.size(); ++i)
        {
            if (conf->routes[i].path == "/") 
            {
                return &conf->routes[i];
            }
        }
    }
    if (!bestLoc)
        return NULL;
    return bestLoc;
}

const Location* findMatchingLocation(const HandleRequest& req, const std::vector<ServerConfig>& _configs, const ServerConfig** conf)
{
    const Location* bestLoc = NULL;
    const ServerConfig* tmp_conf = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _configs.size(); i++)
    {
        tmp_conf = &_configs[i];
        if (!tmp_conf->server_name.empty())
        {
            std::cout << "server config numero " << i << ", a une instruction server_name." << std::endl; //debug
            for (size_t j = 0; j < tmp_conf->server_name.size(); j++)
            {
                std::cout << req.headers.at("Host") << std::endl; // debug
                std::cout << tmp_conf->server_name[j] << std::endl; // debug
                if (req.headers.count("Host") && req.headers.at("Host") == tmp_conf->server_name[j])
                {
                    *conf = tmp_conf;
                    return findMatchingLocationWithName(req, tmp_conf);
                }
            }
        }
    }

    for (size_t i = 0; i < _configs.size(); i++)
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
        for (size_t i = 0; i < _configs.size(); i++)
        {
            tmp_conf = &_configs[i];
            for (size_t j = 0; j < tmp_conf->routes.size(); ++j)
            {
                if (tmp_conf->routes[j].path == "/") 
                {
                    *conf = tmp_conf;
                    return &tmp_conf->routes[j];
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

std::string buildRedirectionResponse(const std::string& location,
									 const std::string& userAgent = "",
									 const std::string& status = "303 See Other") {
	std::ostringstream response;
	std::string body;
	std::string contentType;

	// Détection simple de curl
	bool isCurl = userAgent.find("curl") != std::string::npos;

	if (isCurl) {
		contentType = "text/plain";
		body = "Redirecting to: " + location + "\n";
	} else {
		contentType = "text/html";
		std::ostringstream htmlBody;
		htmlBody << "<html><head><meta http-equiv=\"refresh\" content=\"1; URL=" << location << "\">"
				 << "<title>Redirecting...</title></head><body>"
				 << "<h1>Redirecting...</h1>"
				 << "<p>If you're not redirected automatically, <a href=\"" << location << "\">click here</a>.</p>"
				 << "</body></html>";
		body = htmlBody.str();
	}

	// Construction de la réponse complète
	response << "HTTP/1.1 " << status << "\r\n";
	response << "Location: " << location << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body;

	return response.str();
}

std::string handleDelete(const HandleRequest& req, const std::vector<ServerConfig>& _configs)
{
	std::string cleanPath = req.path;
	size_t qmark = cleanPath.find('?');
	if (qmark != std::string::npos) {
    cleanPath = cleanPath.substr(0, qmark);
	}
	const ServerConfig *conf;
	const Location* loc = findMatchingLocation(req, _configs, &conf);
	if (!loc)
		return buildErrorResponse(404, conf->error_pages);
	if (!isMethodAllowed(loc, loc->methods, "DELETE"))
		return buildErrorResponse(405, conf->error_pages);

	std::string baseRoot = loc->root;
	std::string relPath = cleanPath;

	if (relPath.find(loc->path) == 0)
		relPath = relPath.substr(loc->path.length());
	if (!relPath.empty() && relPath[0] == '/')
		relPath = relPath.substr(1);

	std::string filePath = baseRoot;
	if (!filePath.empty() && filePath.back() != '/')
		filePath += "/";
	filePath += relPath;

	std::cout << "DELETE filePath: " << filePath << std::endl;

	struct stat st;
	if (stat(filePath.c_str(), &st) != 0)
		return buildErrorResponse(404, conf->error_pages);
	if (access(filePath.c_str(), W_OK) != 0)
		return buildErrorResponse(403, conf->error_pages);
	if (remove(filePath.c_str()) != 0)
		return buildErrorResponse(500, conf->error_pages);

	std::string redirectTarget = loc->path;
	std::cout << std::endl << "reqheader is : " << req.headers.at(("User-Agent")) << std::endl;
	return buildRedirectionResponse(redirectTarget, req.headers.at("User-Agent"));
}

std::string HandleGET(const HandleRequest& req, const std::vector<ServerConfig>& _configs)
{
    const ServerConfig* conf;
    const Location* loc = findMatchingLocation(req, _configs, &conf);
    if (!loc) {
        return buildErrorResponse(404, _configs[0].error_pages);
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
        return buildErrorResponse(405, conf->error_pages);
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
        return buildErrorResponse(404, conf->error_pages);
    }
    if (S_ISREG(st.st_mode))
    {
        std::cout << "Is a file: " << filePath << std::endl; // pour debug

        // Vérifie les droits d'accès en lecture
        if (access(filePath.c_str(), R_OK) != 0) {
            return buildErrorResponse(403, conf->error_pages);
        }

        // Détermine le type MIME
        std::string contentType = getMimeType(filePath);

        // Lit le fichier
        std::ifstream file(filePath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return buildErrorResponse(500, conf->error_pages);
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
                return buildErrorResponse(404, conf->error_pages);
            }
            std::ostringstream s;
            s << f.rdbuf();
            std::string body = s.str();
            return buildHttpResponse("200 OK", getMimeType(indexPath), body);
        }
        else if (loc->autoindex)
        {
			std::string html = generateAutoindexHtml(filePath, req.path);
			if (html == "500")
				return buildErrorResponse(500, conf->error_pages);
			if (html == "403")
				return buildErrorResponse(403, conf->error_pages);
			return buildHttpResponse("200 OK", "text/html", html);
        }
    }
    return buildErrorResponse(403, conf->error_pages);
}

std::string HandlePOST(const HandleRequest& req, const std::vector<ServerConfig>& _configs)
{
    /*
    le but:
    consigne hyper large alors on vas decider que si on recois Post avec exec -> upload
    sinon ca deviendra une sorte de echo
    */
       const ServerConfig* conf;
    const Location* loc = findMatchingLocation(req, _configs, &conf);
    if (!loc) {
        return buildErrorResponse(404, _configs[0].error_pages);
    }
    std::cout << "Loc de POST" << loc->path << std::endl; // debug
    // Vérifie la redirection ici
    if (!loc->redirection.empty()) {
        // 302 Found ou 301 Moved Permanently selon ton besoin
        std::string response = "HTTP/1.1 302 Found\r\n";
        response += "Location: " + loc->redirection + "\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: close\r\n\r\n";
        return response;
    }
    if (!isMethodAllowed(loc, loc->methods, "GET"))
    {
        return buildErrorResponse(405, conf->error_pages);
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

    //upload
    if (!loc->upload_store.empty())
    {
        std::cout << "Condition !loc->upload_store.empty()" << std::endl; // pour debug
        if (endsWith(req.path, ".php") && !loc->cgi_extension.empty())
            return exec_cgi(req, *conf, loc, relPath);
        return buildErrorResponse(405, conf->error_pages); // ou 400
    }
    //cgi random
    if (endsWith(req.path, ".php") && !loc->cgi_extension.empty())
    {
        return exec_cgi(req, *conf, loc, relPath);
    }
    // filePath est un fichier donc ambigue donc erreur
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0)
    {
        return buildErrorResponse(404, conf->error_pages);
    }
    if (S_ISREG(st.st_mode))
    {
        buildErrorResponse(405, conf->error_pages);
    }
    //echo
    return buildHtmlEchoResponse(req);

}

std::string SimpleRouter::route(const HandleRequest& req, const std::vector<ServerConfig>& _configs) {
	std::string method = req.method;
	std::string path = req.path;

    printServerConfig(_configs); // pour debug

    if (req.http_version != "HTTP/1.1")
            return buildErrorResponse(505, _configs[0].error_pages);
    if (method == "GET")
        return HandleGET(req, _configs);
    if (method == "POST")
        return HandlePOST(req, _configs);
        //Route pour la page d'accueil (album)
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

	// if (method == "POST" && endsWith(path, ".php")) // surement changer la logique en fonction des location mais pour l'instant c'est pour test
	// {
	// 	return exec_cgi(req, _configs);
	// 	//return buildHttpResponse("200 OK", "text/plain", "PHP script executed successfully.");
	// }

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