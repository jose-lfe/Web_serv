#include "CGI.hpp"

std::vector<std::string> buildCgiEnv(const handleRequest& req, const std::string& scriptPath, const Location *loc, const ServerConfig& configs) {
    std::vector<std::string> env;

    if (configs.cpy_envp)
    {
        for (int i = 0; configs.cpy_envp[i]; ++i)
        {
            env.push_back(std::string(configs.cpy_envp[i]));
        }
    }

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

std::string get_pwd_path(const ServerConfig& configs)
{
    char **env = configs.cpy_envp;
    if (!env)
        return "";

    for (int i = 0; env[i]; ++i) {
        std::string var(env[i]);
        if (var.find("PWD=") == 0)
            return var.substr(4);
    }
    return "";
}

std::string exec_cgi(const handleRequest& req, const ServerConfig& configs, const Location *loc, std::string relPath)
//std::string exec_cgi(const handleRequest& req, const std::vector<ServerConfig>& configs)
{
    std::cout << "inside exec_cgi" << std::endl; // debug
    std::string scriptPath = get_pwd_path(configs) + "/www/" + relPath;
    std::string interpreter = loc->cgi_path;
    std::string newbody;
    std::cout << "script path: " << scriptPath << std::endl; // debug


    std::vector<std::string> env_vec = buildCgiEnv(req, scriptPath, loc, configs);
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
            if (!contentType.empty() && contentType[contentType.size() - 1] == '\r')
                contentType.erase(contentType.size() - 1);
            break;
        }
    }
    return buildHttpResponse("200 OK", contentType, body);
}
