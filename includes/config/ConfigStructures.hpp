/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructures.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:38:58 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/19 23:27:56 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_STRUCTURES_HPP
#define CONFIG_STRUCTURES_HPP

#include <string>
#include <vector>
#include <map>

struct ConfigContext
{
	ConfigContext();
	~ConfigContext();
	
	std::string root;
	std::vector<std::string> index;
	std::string clientMaxBodySize;
	int autoIndex;
	std::map<int, std::string> errorPages;
	std::string cgiBinPath;
	void inheritFrom(const ConfigContext& parent);
	void applyDefaults();
};

struct ListenConfig
{
	ListenConfig();
	~ListenConfig();
	int port;
	std::string host;
	bool operator==(const ListenConfig& other) const;
};

struct LocationConfig
{
	LocationConfig();
	~LocationConfig();
	std::string path;
	ConfigContext ctx;
	std::vector<std::string> allowedMethods;
	int redirectCode;
	std::string redirectUrl;
	int uploadEnabled;
	std::string uploadPath;
	int cgiEnabled;
	std::vector<std::string> cgiExtensions;
	void applyDefaults();
	bool operator()(class ConfigParser* parser, const std::string& directive);
	
};

struct ServerConfig
{
	ServerConfig();
	~ServerConfig();
	std::vector<ListenConfig> listen;
	std::vector<std::string> serverNames;
	ConfigContext ctx;
	std::vector<LocationConfig> locations;
	void applyDefaults();
	const LocationConfig* findLocationByUri(const std::string& url) const;
	bool operator()(class ConfigParser* parser, const std::string& directive);
};

struct HttpConfig
{
	HttpConfig();
	~HttpConfig();
	ConfigContext ctx;
	std::vector<ServerConfig> servers;
	void createDefaultConfig();
	const ServerConfig* findServerByHost(const std::string& hostHeader,
																	const std::string& localIp, const int localPort) const;
	bool operator()(class ConfigParser* parser, const std::string& directive);
};

#endif
