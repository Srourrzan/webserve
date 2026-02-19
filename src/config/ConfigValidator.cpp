/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 23:47:58 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/19 23:36:01 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"

ConfigValidator::ConfigValidator() {}

ConfigValidator::~ConfigValidator() {}

static void compareServers(const ServerConfig& s1, const ServerConfig& s2)
{
    if (ConfigValidator::hasCommonElement(s1.serverNames, s2.serverNames) && 
        ConfigValidator::hasCommonElement(s1.listen, s2.listen))
        throw std::runtime_error("Duplicate server: same listen address and same server_name");
}

void ConfigValidator::checkValue(const ConfigTokenizer& ConfigTokenizer, const std::string& directive)
{
    if (!ConfigTokenizer.hasMore() || ConfigValidator::isBlock(ConfigTokenizer.peek()))
        throw std::runtime_error(directive + " directive requires a value (missing value before ';')");
    if (isReserved(ConfigTokenizer.peek()))
        throw std::runtime_error("Cannot use directive '" +
             ConfigTokenizer.peek() + "' as value for " + directive);
}

bool ConfigValidator::isValidMethod(const std::string& method)
{
    return (method == "GET" || method == "POST" || method == "DELETE");
}

bool ConfigValidator::isValidStatus(int code)
{
    return (code == 200 ||                                    
            code == 300 || code == 302 ||                   
            code == 400 || code == 404 || code == 405 ||    
            code == 411 || code == 413 || code == 414 ||                    
            code == 501 || code == 505);                   
}

bool ConfigValidator::isValidRedirectCode(int code)
{
    return (code == 300 || code == 301 || code == 302);
}

bool ConfigValidator::isReserved(const std::string& word)
{
    static const char* reserved[] = {
        "http", "server", "location", "listen", "server_name", "root",
        "index", "error_page", "client_max_body_size", "autoindex", 
        "methods", "return", "upload", "upload_path", "cgi", "cgi_ext", "cgi_bin_path"
    };
    static const size_t count = sizeof(reserved) / sizeof(reserved[0]);
    
    for (size_t i = 0; i < count; i++)
        if (word == reserved[i])
            return true;
    return false;
}

bool ConfigValidator::isBlock(const std::string& token)
{
    return (token == ";" || token == "{" || token == "}");
}

bool ConfigValidator::isDigits(const std::string& str)
{
    if (str.empty())
        return false;
    for (size_t i = 0; i < str.length(); i++)
    {
        if (!std::isdigit(static_cast<unsigned char>(str[i])))
            return false;
    }
    return true;
}

bool ConfigValidator::isValidIP(const std::string& ip)
{
    size_t dots = 0;
    size_t start = 0;

    if (ip.empty())
        return false;
    for (size_t i = 0; i <= ip.length(); i++)
    {
        if (i == ip.length() || ip[i] == '.')
        {
            if (i == start)
                return false;
            std::string octet = ip.substr(start, i - start);
            
            if (octet.length() > 3 || !isDigits(octet))
                return false;
            int val = std::atoi(octet.c_str());
            if (val < 0 || val > 255)
                return false;
            if (i < ip.length())
                dots++;
            start = i + 1;
        }
    }
    return dots == 3;
}

bool ConfigValidator::isValidPort(const std::string& port)
{
    if (!isDigits(port))
        return false;
    
    int p = std::atoi(port.c_str());
    return (p > 0 && p <= 65535);
}

bool ConfigValidator::isValidHostname(const std::string& hostname)
{
	size_t start = 0;
	size_t pos;
	if (hostname.empty() || hostname.length() > 253)
		return false;
	if (hostname == "localhost")
		return true;
	if (hostname[0] == '.' || hostname[hostname.length() - 1] == '.')
		return false;
	while (true)
	{
		pos = hostname.find('.', start);
		size_t end = (pos == std::string::npos) ? hostname.length() : pos;
		std::string label = hostname.substr(start, end - start);
		if (label.empty() || label.length() > 63)
			return false;
		if (label[0] == '-' || label[label.length() - 1] == '-')
			return false;
		for (size_t j = 0; j < label.length(); j++)
		{
			char c = label[j];
			if (!std::isalnum(c) && c != '-')
				return false;
		}
		if (pos == std::string::npos)
			break;
		start = pos + 1;
	}
	return true;
}

bool ConfigValidator::isValidString(const std::string& value, const std::string& allowedChars)
{
	if (value.empty())
		return false;
	for (size_t i = 0; i < value.length(); i++)
	{
		char c = value[i];
		if (std::isalnum(static_cast<unsigned char>(c)))
			continue;
		if (allowedChars.find(c) != std::string::npos)
			continue;
		return false;
	}
	return true;
}

void ConfigValidator::validateMethod(const std::string& method)
{
    if (isReserved(method))
        throw std::runtime_error("Cannot use directive '" + method + "' as HTTP method");
    if (!isValidMethod(method))
        throw std::runtime_error("Invalid HTTP method: " + method);
}
        
void ConfigValidator::validateLocation(const LocationConfig& loc)
{
	if (loc.path.empty())
		throw std::runtime_error("Location must have a path");
	if (loc.redirectCode != 0)
		return;
	validateContext(loc.ctx);
	for (size_t i = 0; i < loc.allowedMethods.size(); i++)
		validateMethod(loc.allowedMethods[i]);
	if (loc.uploadEnabled && !isValidString(loc.uploadPath, "_-./"))
		throw std::runtime_error("Invalid upload_path: " + loc.uploadPath);    
	if (loc.cgiEnabled && loc.cgiExtensions.empty())
		throw std::runtime_error("Location with cgi enabled must have cgi_ext");
}

void ConfigValidator::validateServer(const ServerConfig& srv)
{
    for (size_t i = 0; i < srv.serverNames.size(); i++)
    {
        const std::string& name = srv.serverNames[i];
        
        if (!isValidString(name, ".-*"))
            throw std::runtime_error("Invalid server_name: " + name);
        for (size_t j = 0; j < name.length(); j++)
        {
            if (name[j] == '*' && j != 0 && j != name.length() - 1)
                throw std::runtime_error("Invalid server_name: " + name +
                     " (* only at start or end)");
        }
    }    
    validateContext(srv.ctx);
    for (size_t i = 0; i < srv.locations.size(); i++)
        validateLocation(srv.locations[i]);
}

void ConfigValidator::checkDupLocations(const HttpConfig& config)
{
    for (size_t i = 0; i < config.servers.size(); i++)
    {
        const ServerConfig& srv = config.servers[i];
        std::map<std::string, bool> paths;
        
        for (size_t k = 0; k < srv.locations.size(); k++)
        {
            std::string path = srv.locations[k].path;
            if (paths.find(path) != paths.end())
                throw std::runtime_error("Duplicate location: " + path);
            paths[path] = true;
        }
    }
}

void ConfigValidator::checkDuplicates(const HttpConfig& config)
{
    checkDupLocations(config);
    compareAllPairs(config.servers, compareServers);
}

void ConfigValidator::validateContext(const ConfigContext& ctx)
{
    if (!ctx.root.empty() && !isValidString(ctx.root, "_-./"))
        throw std::runtime_error("Invalid root: " + ctx.root);
    for (size_t i = 0; i < ctx.index.size(); i++)
    {
        if (!isValidString(ctx.index[i], "_-./"))
            throw std::runtime_error("Invalid index: " + ctx.index[i]);
    }            
    for (std::map<int, std::string>::const_iterator it = ctx.errorPages.begin();
         it != ctx.errorPages.end(); ++it)
    {
        if (!isValidString(it->second, "_-./"))
            throw std::runtime_error("Invalid error_page path: " + it->second);
    }            
    if (!ctx.cgiBinPath.empty() && !isValidString(ctx.cgiBinPath, "_-./"))
        throw std::runtime_error("Invalid cgi_bin_path: " + ctx.cgiBinPath);
}

void ConfigValidator::validate(const HttpConfig& config)
{
    if (config.servers.empty())
        throw std::runtime_error("Configuration must have at least one server");    
    validateContext(config.ctx);
    for (size_t i = 0; i < config.servers.size(); i++)
        validateServer(config.servers[i]);
    checkDuplicates(config);
}
