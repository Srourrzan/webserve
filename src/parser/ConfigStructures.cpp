/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructures.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:44:23 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/20 00:30:33 by dikhalil         ###   ########.fr       */
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
    if (errorPages.find(403) == errorPages.end())
        errorPages[403] = root + "/error_pages/403.html";
    if (errorPages.find(404) == errorPages.end())
        errorPages[404] = root + "/error_pages/404.html";
    if (errorPages.find(405) == errorPages.end())
        errorPages[405] = root + "/error_pages/405.html";
    if (errorPages.find(500) == errorPages.end())
        errorPages[500] = root + "/error_pages/500.html";
    if (errorPages.find(502) == errorPages.end())
        errorPages[502] = root + "/error_pages/502.html";
    if (errorPages.find(504) == errorPages.end())
        errorPages[504] = root + "/error_pages/504.html";
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

bool LocationConfig::operator()(ConfigParser* parser, const std::string& directive)
{
    return parser->parseLocDirective(directive, *this);
}
