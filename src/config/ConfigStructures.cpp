/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructures.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:44:23 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/14 18:28:31 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigStructures.hpp"
#include "ConfigParser.hpp"

ConfigContext::ConfigContext() : autoIndex(-1) {}

void ConfigContext::inheritFrom(const ConfigContext& parent)
{
    if (root.empty())
        root = parent.root;
    if (index.empty())
        index = parent.index;
    if (clientMaxBodySize.empty())
        clientMaxBodySize = parent.clientMaxBodySize;
    if (autoIndex == -1)
        autoIndex = parent.autoIndex;
    if (cgiBinPath.empty())
        cgiBinPath = parent.cgiBinPath;
    for (std::map<int, std::string>::const_iterator it = parent.errorPages.begin(); 
         it != parent.errorPages.end(); ++it)
        if (errorPages.find(it->first) == errorPages.end())
            errorPages[it->first] = it->second;
}

void ConfigContext::applyDefaults()
{
    if (root.empty())
        root = "./www";
    if (index.empty())
        index.push_back("/html/index.html");
    if (clientMaxBodySize.empty())
    {
        std::ostringstream oss;
        oss << (1024 * 1024);
        clientMaxBodySize = oss.str();
    }
    if (autoIndex == -1)
        autoIndex = 0;    
    static const int supportedErrorCodes[] = {
        400, 403, 404, 405, 409, 413, 414, 500, 501, 505};
    for (size_t i = 0; i < sizeof(supportedErrorCodes)/sizeof(supportedErrorCodes[0]); ++i) {
        int code = supportedErrorCodes[i];
        if (errorPages.find(code) == errorPages.end()) {
            std::ostringstream oss;
            oss  << "/html/error_pages/" << code << ".html";
            errorPages[code] = oss.str();
        }
    }
}

ListenConfig::ListenConfig() : port(8080)
{
    host = "0.0.0.0";
}

bool ListenConfig::operator==(const ListenConfig& other) const
{
    return (host == other.host && port == other.port);
}

LocationConfig::LocationConfig() : redirectCode(0), uploadEnabled(0), cgiEnabled(0) {}

ServerConfig::ServerConfig()
{
    ListenConfig defaultListen;
    listen.push_back(defaultListen);
}

HttpConfig::HttpConfig() {}

void LocationConfig::applyDefaults()
{
    ctx.applyDefaults();
    
    if (allowedMethods.empty())
    {
        allowedMethods.push_back("GET");
        allowedMethods.push_back("POST");
        allowedMethods.push_back("DELETE");
    }
}

void ServerConfig::applyDefaults()
{
    ctx.applyDefaults();
    
    if (locations.empty())
    {
        LocationConfig defaultLocation;
        defaultLocation.path = "/";
        locations.push_back(defaultLocation);
    }
    for (size_t i = 0; i < locations.size(); i++)
    {
        locations[i].ctx.inheritFrom(ctx);
        locations[i].applyDefaults();
    }
    if (serverNames.empty())
        serverNames.push_back("localhost");
}

void HttpConfig::createDefaultConfig()
{
    ServerConfig defaultServer;
    
    defaultServer.applyDefaults();
    servers.push_back(defaultServer);
}

bool HttpConfig::operator()(ConfigParser* parser, const std::string& directive)
{
    if (directive == "server")
    {
        parser->parseServer();
        return true;
    }
    return false;
}

bool ServerConfig::operator()(ConfigParser* parser, const std::string& directive)
{
    if (directive == "listen")
    {
        parser->parseListen(*this);
        return true;
    }
    else if (directive == "server_name")
    {
        parser->parseString("server_name", &serverNames);
        return true;
    }
    else if (directive == "location")
    {
        parser->parseLocation(*this);
        return true;
    }
    return false;
}

const ServerConfig* HttpConfig::findServerByHost(const std::string& hostHeader,
    const std::string& localIp, const int localPort) const
{
    std::string hostname = hostHeader;
    size_t colonPos = hostHeader.find(':');
    if (colonPos != std::string::npos)
        hostname = hostHeader.substr(0, colonPos);
    ServerConfig* defServer = NULL;
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig& srv = servers[i];      
        for (size_t j = 0; j < srv.listen.size(); ++j)
        {
            const ListenConfig& lst = srv.listen[j];
            if (lst.port != localPort)
                continue;
            if (lst.host != "0.0.0.0" && lst.host != localIp)
                continue;
            if (!defServer)
                defServer = const_cast<ServerConfig*>(&srv);
            for (size_t k = 0; k < srv.serverNames.size(); ++k)
            {
                if (srv.serverNames[k] == hostname)
                    return const_cast<ServerConfig*>(&srv);
            }
        }
    }
    if (defServer)
        return defServer;
    return NULL;
}

const LocationConfig *ServerConfig::findLocationByUri(const std::string &uri) const
{
	std::string cleanUri = uri;
	size_t queryPos = cleanUri.find('?');
	if (queryPos != std::string::npos)
		cleanUri = cleanUri.substr(0, queryPos);

	const LocationConfig *bestMatch = NULL;
	const LocationConfig *defaultMatch = NULL;
	size_t bestLength = 0;

	for (size_t i = 0; i < locations.size(); i++)
	{
		const LocationConfig& loc = locations[i];
		const std::string& path = loc.path;
		if (path == "/")
				defaultMatch = &loc;
		if (cleanUri == path)
				return &loc;
		if (cleanUri.compare(0, path.length(), path) == 0 && path.length() > bestLength)
		{
			bestLength = path.length();
			bestMatch = &loc;
		}
	}
	if (bestMatch)
	{
		return bestMatch;
	}
	if (defaultMatch)
	{
		return defaultMatch;
	}
	return NULL;
}

bool LocationConfig::operator()(ConfigParser* parser, const std::string& directive)
{
    return parser->parseLocDirective(directive, *this);
}
