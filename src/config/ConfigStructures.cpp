/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructures.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:44:23 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/27 01:17:03 by dikhalil         ###   ########.fr       */
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
        index.push_back("index.html");
    if (clientMaxBodySize.empty())
    {
        std::ostringstream oss;
        oss << (1024 * 1024);
        clientMaxBodySize = oss.str();
    }
    if (autoIndex == -1)
        autoIndex = 0;    
    if (errorPages.find(400) == errorPages.end())
        errorPages[400] = root + "/error_pages/400.html";
    if (errorPages.find(404) == errorPages.end())
        errorPages[404] = root + "/error_pages/404.html";
    if (errorPages.find(405) == errorPages.end())
        errorPages[405] = root + "/error_pages/405.html";
    if (errorPages.find(411) == errorPages.end())
        errorPages[411] = root + "/error_pages/411.html";
    if (errorPages.find(413) == errorPages.end())
        errorPages[413] = root + "/error_pages/413.html";
    if (errorPages.find(414) == errorPages.end())
        errorPages[414] = root + "/error_pages/414.html";
    if (errorPages.find(501) == errorPages.end())
        errorPages[501] = root + "/error_pages/501.html";
    if (errorPages.find(505) == errorPages.end())
        errorPages[505] = root + "/error_pages/505.html";
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
    LocationConfig defaultLocation;
    
    defaultServer.locations.push_back(defaultLocation);
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

ServerConfig* HttpConfig::findServerByHost(const std::string& hostHeader, std::string& localIp, int localPort) const
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
            if (lst.host != localIp && lst.host != "0.0.0.0")
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

const LocationConfig &ServerConfig::findLocationByUri(const std::string& uri) const
{
    const LocationConfig *bestMatch = NULL;
    const LocationConfig *defaultMatch = NULL;
    size_t bestLength = 0;
    
    for (size_t i = 0; i < locations.size(); i++)
    {
        const LocationConfig& loc = locations[i];
        const std::string& path = loc.path;
        if (path == "/")
            defaultMatch = &loc;
        if (uri.compare(0, path.length(), path) == 0 && path.length() > bestLength)
        {
            if (uri.length() != path.length() && uri[path.length()] != '/')
                continue;
            bestLength = path.length();
            bestMatch = &loc;
        }
    }
    if (!bestMatch)
        return *defaultMatch;
    return *bestMatch;
}

bool LocationConfig::operator()(ConfigParser* parser, const std::string& directive)
{
    return parser->parseLocDirective(directive, *this);
}
