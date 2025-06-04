#include "SimpleRouter.hpp"
#include <string>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>


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
    std::string scriptPath = "/home/jose-lfe/projects/web/work/www" + req.path;
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

// std::string exec_cgi(const HandleRequest& req)
// {
//     std::string scriptPath = "/home/jose-lfe/projects/web/work/www" + req.path;
//     std::string interpreter = "/usr/bin/php";
// 	std::string newbody;
// 	int fdin = dup(STDIN_FILENO);
// 	int fdout = dup(STDOUT_FILENO);

//     std::vector<std::string> env_vec = buildCgiEnv(req, scriptPath);
//     std::vector<char*> envp;
//     for (size_t i = 0; i < env_vec.size(); ++i) {

//         envp.push_back(const_cast<char*>(env_vec[i].c_str()));
//         std::cout << "env_vec[" << i << "]: " << env_vec[i] << std::endl; // pour debug
//     }
//     envp.push_back(NULL);

//     char* argv[3];
//     argv[0] = const_cast<char*>(interpreter.c_str());
//     argv[1] = const_cast<char*>(scriptPath.c_str());
//     argv[2] = NULL;

// 	FILE *fIn = tmpfile();
// 	FILE *fOut = tmpfile();
// 	long fdIn = fileno(fIn);
// 	long fdOut = fileno(fOut);
// 	int ret = 1;

//     std::cerr << "Body size: " << req.body.size() << " / Content-Length: " << getHeaderValue(req.headers, "Content-Length") << std::endl; // pour debug

//     write(fdIn, req.body.c_str(), req.body.size());
// 	lseek(fdIn, 0, SEEK_SET);

//     std::ofstream debugOut("/tmp/debug_cgi_body.bin", std::ios::binary);
//     debugOut.write(req.body.c_str(), req.body.size());
//     debugOut.close();

//     pid_t pid = fork();
//     if (pid < 0) {
//         perror("fork");
//         return buildHttpResponse("500 Internal Server Error", "text/plain", "Fork error");
//     }

//     if (pid == 0) {
//         // Enfant
//         dup2(fdIn, STDIN_FILENO);
// 		dup2(fdOut, STDOUT_FILENO);
// 		execve(interpreter.c_str(), argv, envp.data());
// 		std::cerr << "Execution failed" << std::endl;
// 		write(STDOUT_FILENO, "Code 500\n", 10);
// 		exit(1);
//     }
//     char buffer[1000];
//     waitpid(-1, NULL, 0);
//     lseek(fdOut, 0, SEEK_SET);
//     ret = 1;
//     while (ret > 0)
//     {
//         memset(buffer, 0, 1000);
//         ret = read(fdOut, buffer, 1000 - 1);
//         newbody += buffer;
//     }

//     // Parent
// 	dup2(fdin, STDIN_FILENO);
// 	dup2(fdout, STDOUT_FILENO);
// 	fclose(fIn);
// 	fclose(fOut);
// 	close(fdIn);
// 	close(fdOut);
// 	close(fdin);
// 	close(fdout);

//     std::string headers, body;
//     size_t pos = newbody.find("\r\n\r\n");
//     if (pos != std::string::npos) {
//         headers = newbody.substr(0, pos);
//         body = newbody.substr(pos + 4);
//     } else {
//         body = newbody;
//     }

//     std::string contentType = "text/html";
//     std::istringstream hstream(headers);
//     std::string line;
//     while (std::getline(hstream, line)) {
//         if (line.find("Content-Type:") == 0) {
//             contentType = line.substr(13);
//             while (!contentType.empty() && (contentType[0] == ' ' || contentType[0] == '\t'))
//                 contentType.erase(0, 1);
//             if (!contentType.empty() && contentType.back() == '\r')
//                 contentType.pop_back();
//             break;
//         }
//     }
//     return buildHttpResponse("200 OK", contentType, body);
// }

// std::string exec_cgi(const HandleRequest& req)
// {
//     std::string scriptPath = "www" + req.path; // adapte selon ta logique
//     std::string interpreter = "/usr/bin/php";  // adapte selon ta config

//     std::vector<std::string> env_vec = buildCgiEnv(req, scriptPath);
//     std::vector<char*> envp;
//     for (size_t i = 0; i < env_vec.size(); ++i)
//         envp.push_back(const_cast<char*>(env_vec[i].c_str()));
//     envp.push_back(NULL);

//     char* argv[3];
//     argv[0] = const_cast<char*>(interpreter.c_str());
//     argv[1] = const_cast<char*>(scriptPath.c_str());
//     argv[2] = NULL;

//     int in_pipe[2], out_pipe[2];
//     pipe(in_pipe);
//     pipe(out_pipe);

//     pid_t pid = fork();
//     if (pid == 0) {
//         dup2(in_pipe[0], 0);
//         dup2(out_pipe[1], 1);
//         close(in_pipe[1]);
//         close(out_pipe[0]);
//         execve(interpreter.c_str(), argv, envp.data());
//         perror("execve");
//         exit(1);
//     }

//     close(in_pipe[0]);
//     close(out_pipe[1]);

//     if (req.method == "POST" && !req.body.empty()) {
//         write(in_pipe[1], req.body.c_str(), req.body.size());
// 		std::cout << "POST body sent to CGI script: " << req.body << std::endl; // pour debug
//     }
//     close(in_pipe[1]);

//     std::string cgi_output;
//     char buffer[4096];
//     ssize_t n;
//     while ((n = read(out_pipe[0], buffer, sizeof(buffer))) > 0) {
//         cgi_output.append(buffer, n);
//     }
// 	std::cout << "CGI output received: " << cgi_output << std::endl; // pour debug
//     close(out_pipe[0]);
//     int status;
//     waitpid(pid, &status, 0);

//     // --- PARSING CGI OUTPUT ---
//     std::string headers, body;
//     size_t pos = cgi_output.find("\r\n\r\n");
//     if (pos != std::string::npos) {
//         headers = cgi_output.substr(0, pos);
//         body = cgi_output.substr(pos + 4);
//     } else {
//         // fallback: tout dans le body
//         body = cgi_output;
//     }

//     // Cherche le Content-Type dans les headers CGI
//     std::string contentType = "text/html";
//     std::istringstream hstream(headers);
//     std::string line;
//     while (std::getline(hstream, line)) {
//         if (line.find("Content-Type:") == 0) {
//             contentType = line.substr(13);
//             // Nettoie les espaces et \r éventuels
//             while (!contentType.empty() && (contentType[0] == ' ' || contentType[0] == '\t'))
//                 contentType.erase(0, 1);
//             if (!contentType.empty() && contentType[contentType.size()-1] == '\r')
//                 contentType.erase(contentType.size()-1);
//             break;
//         }
//     }

//     // Génère une vraie réponse HTTP
//     return buildHttpResponse("200 OK", contentType, body);
// }

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

std::string SimpleRouter::route(const HandleRequest& req) {
	std::string method = req.method;
	std::string path = req.path;
	// Route pour la page d'accueil (album)
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