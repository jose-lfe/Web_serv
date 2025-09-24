#include "handleMethod.hpp"
#include "handleGallery.hpp"


std::string handleDelete(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port)
{
	std::string cleanPath = req.path;
	size_t qmark = cleanPath.find('?');
	if (qmark != std::string::npos) {
    cleanPath = cleanPath.substr(0, qmark);
	}
	const ServerConfig *conf;
	const Location* loc = findMatchingLocation(req, _configs, &conf, port);
	if (!loc)
		return buildErrorResponse(404, conf->error_pages);
	if (!isMethodAllowed(loc, loc->methods, "DELETE"))
		return buildErrorResponse(405, conf->error_pages);


    std::string deletePrefix = "/delete/";
    if (cleanPath.find(deletePrefix) == 0) cleanPath = cleanPath.substr(deletePrefix.length());
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
	std::cout << "JOSE " << redirectTarget << std::endl;
	std::cout << std::endl << "reqheader is : " << req.headers.at(("User-Agent")) << std::endl;
	return buildRedirectionResponse(redirectTarget, req.headers.at("User-Agent"));
}

std::string handleGET(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port)
{
    const ServerConfig* conf;
    const Location* loc = findMatchingLocation(req, _configs, &conf, port);
    if (!loc) {
		std::cout << "jose est sur" << std::endl;
        return buildErrorResponse(404, _configs[0].error_pages);
    }
    if (!loc->redirection.empty()) {
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
    std::string baseRoot;
    baseRoot = loc->root;
    std::string relPath = req.path;

    //get rid of queryString for now
    std::string::size_type pos = relPath.find('?');
    if (pos != std::string::npos)
    {
        relPath = relPath.substr(0, pos);
    }

    if (relPath.find(loc->path) == 0)
        relPath = relPath.substr(loc->path.length());
    if (!relPath.empty() && relPath[0] == '/')
        relPath = relPath.substr(1);
    std::cout << std::endl << relPath << std::endl << std::endl;

    std::string filePath = baseRoot;
    if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
        filePath += "/";
    filePath += relPath;
    std::cout << "GET filePath: " << filePath << std::endl; // pour debug

    if (filePath.find("template/gallery.html") != std::string::npos)
    {
        std::string contentType = getMimeType(filePath);
        return buildHttpResponse("200 OK", contentType, renderGallery(filePath, loc->root + "photos"), true);
    }

    if (filePath == "www/kill")
        throw std::runtime_error("Serveur killed by client");
    
    struct stat st;
    if (stat(filePath.c_str(), &st) != 0) {
        return buildErrorResponse(404, conf->error_pages);
    }
    if (S_ISREG(st.st_mode))
    {
        std::cout << "Is a file: " << filePath << std::endl; // pour debug

        if (access(filePath.c_str(), R_OK) != 0) {
            return buildErrorResponse(403, conf->error_pages);
        }

        std::string contentType = getMimeType(filePath);

        //cgi GET
        if (contentType == "application/x-httpd-php" && loc->cgi_extension == ".php")
        {
            return exec_cgi(req, *conf, loc, relPath);
        }

        std::ifstream file(filePath.c_str(), std::ios::binary);
        if (!file.is_open()) {
            return buildErrorResponse(500, conf->error_pages);
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string body = ss.str();

        return buildHttpResponse("200 OK", contentType, body, true);
    }
    else if (S_ISDIR(st.st_mode))
    {
        std::cout << "Is a directory: " << filePath << std::endl; //pour debug
        if (!loc->index.empty())
        {
            std::string indexPath = filePath;
            if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
                indexPath += "/";
            indexPath += loc->index;
            std::cout << "Index path: " << indexPath << std::endl; //pour debug
			if (indexPath.find("template/gallery.html") != std::string::npos)
    		{
        		std::string contentType = getMimeType(indexPath);
        		return buildHttpResponse("200 OK", contentType, renderGallery(indexPath, loc->root + "photos"), true);
    		}

            std::ifstream f(indexPath.c_str());
            if (!f) {
                return buildErrorResponse(404, conf->error_pages);
            }
            std::ostringstream s;
            s << f.rdbuf();
            std::string body = s.str();
            return buildHttpResponse("200 OK", getMimeType(indexPath), body, true);
        }
        else if (loc->autoindex)
        {
			std::string html = generateAutoindexHtml(filePath, req.path);
			if (html == "500")
				return buildErrorResponse(500, conf->error_pages);
			if (html == "403")
				return buildErrorResponse(403, conf->error_pages);
			return buildHttpResponse("200 OK", "text/html", html, true);
        }
    }
    return buildErrorResponse(403, conf->error_pages);
}

std::string handlePOST(const handleRequest& req, const std::vector<ServerConfig>& _configs, int port)
{
    /*
    le but:
    consigne hyper large alors on vas decider que si on recois Post avec exec -> upload
    sinon ca deviendra une sorte de echo
    */
       const ServerConfig* conf;
    const Location* loc = findMatchingLocation(req, _configs, &conf, port);

    std::cout << "Loc de POST" << loc->path << std::endl; // debug

    std::map<std::string, std::string>::const_iterator it = req.headers.find("Content-Length");
    if (it != req.headers.end() && loc->client_max_body_size != 0)
    {
        size_t content_length = std::strtoul(it->second.c_str(), NULL, 10);
        if (content_length > loc->client_max_body_size)
        {
            return buildErrorResponse(413, conf->error_pages);
        }
    }

    if (!loc->redirection.empty()) {
        std::string response = "HTTP/1.1 302 Found\r\n";
        response += "Location: " + loc->redirection + "\r\n";
        response += "Content-Length: 0\r\n";
        response += "Connection: close\r\n\r\n";
        return response;
    }
    if (!isMethodAllowed(loc, loc->methods, "POST"))
    {
        return buildErrorResponse(405, conf->error_pages);
    }
    std::string baseRoot;
    baseRoot = loc->root;
    std::string relPath = req.path;
    if (relPath.find(loc->path) == 0)
        relPath = relPath.substr(loc->path.length());
    if (!relPath.empty() && relPath[0] == '/')
        relPath = relPath.substr(1);
    std::cout << std::endl << relPath << std::endl << std::endl;

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
    //std::cout << "random cgi : enwith : " << endsWith(req.path, ".php") << " loc : " << !loc->cgi_extension.empty() << std::endl; //debug
    if (endsWith(req.path, ".php") && !loc->cgi_extension.empty())
    {
        return exec_cgi(req, *conf, loc, relPath);
    }

    std::cout << "echo ici" << std::endl;
    return buildHtmlEchoResponse(req);

}
