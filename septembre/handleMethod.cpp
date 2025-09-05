#include "handleMethod.hpp"

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
	if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
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
            if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
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
    // struct stat st;
    // if (stat(filePath.c_str(), &st) != 0)
    // {
    //     return buildErrorResponse(404, conf->error_pages);
    // }
    // if (S_ISREG(st.st_mode))
    // {
    //     buildErrorResponse(405, conf->error_pages);
    // }
    //echo
    std::cout << "echo ici" << std::endl;
    return buildHtmlEchoResponse(req);

}
