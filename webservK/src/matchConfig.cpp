#include "matchConfig.hpp"

const Location* findMatchingLocationWithName(const handleRequest &req, const ServerConfig* conf)
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

const Location* findMatchingLocation(const handleRequest& req, const std::vector<ServerConfig>& _configs, const ServerConfig** conf, int port)
{

    const Location* bestLoc = NULL;
    const ServerConfig* tmp_conf = NULL;
    size_t bestLen = 0;

    for (size_t i = 0; i < _configs.size(); i++)
    {
        tmp_conf = &_configs[i];
		for (size_t n = 0; n < tmp_conf->port.size(); ++n) 
		if (tmp_conf->port[n] == port)
		{
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
	}
    for (size_t i = 0; i < _configs.size(); i++)
    {
        tmp_conf = &_configs[i];
		for (size_t n = 0; n < tmp_conf->port.size(); ++n) 
		{
			if (tmp_conf->port[n] == port)
			{
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
		}
    }

    // Si aucune location ne matche, cherche la location "/"
    if (!bestLoc) 
    {
        for (size_t i = 0; i < _configs.size(); i++)
        {
            tmp_conf = &_configs[i];
			for (size_t n = 0; n < tmp_conf->port.size(); ++n) 
				if (tmp_conf->port[n] == port)
				{
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
    }

    if (!bestLoc)
	{
		*conf = &_configs[0];
		const Location res(_configs[0]);
		const Location *resptr = &res;
        return resptr;
	}
    
    return bestLoc;
}
