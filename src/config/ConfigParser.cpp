/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:36:34 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/27 00:41:35 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigParser::ConfigParser() {}

void ConfigParser::parse(const std::string& filename)
{
    tokenizer.tokenizeFile(filename);
    
    if (tokenizer.getTokens().empty())
        throw std::runtime_error("Config file is empty");
    std::string firstToken = tokenizer.peek();
    
    if (firstToken == "http")
        parseHttp();
    else if (firstToken == "server")
    {
        while (tokenizer.hasMore())
        {
            if (tokenizer.peek() == "server")
                parseServer();
            else
                throw std::runtime_error("Expected 'server' block");
        }
    }
    else
        throw std::runtime_error("Config must start with 'http' or 'server' block");
    if (config.servers.empty())
        throw std::runtime_error("Config must contain at least one server block");
    if (tokenizer.hasMore())
    {
        std::string extraToken = tokenizer.peek();
        if (extraToken == "}")
            throw std::runtime_error("Unexpected closing brace '}'");
        else if (extraToken == "{")
            throw std::runtime_error("Unexpected opening brace '{'");
        else
            throw std::runtime_error("Unexpected token after config: '" + extraToken + "'");
    }
    applyDefaults();
    validator.validate(config);
}

bool ConfigParser::parseContext(const std::string& directive, ConfigContext& ctx)
{
    if (directive == "root" || directive == "cgi_bin_path")
        parseString(directive, NULL, (directive == "root" ? &ctx.root : &ctx.cgiBinPath));
    else if (directive == "index")
        parseString("index", &ctx.index);
    else if (directive == "client_max_body_size")
        parseBodySize(ctx);
    else if (directive == "autoindex")
        parseBool("autoindex", ctx.autoIndex);
    else if (directive == "error_page")
        parseErrorPage(ctx);
    else
        return false;
    return true;
}

void ConfigParser::parseHttp()
{
    tokenizer.expect("http");
    parseBlock(config.ctx, config);
    
    if (config.servers.empty())
        throw std::runtime_error("HTTP block must contain at least one server block");
}

void ConfigParser::parseServer()
{
    ServerConfig server;
    tokenizer.expect("server");
    parseBlock(server.ctx, server);
    
    config.servers.push_back(server);
}

void ConfigParser::parseLocation(ServerConfig& server)
{
    LocationConfig location;
    
    tokenizer.expect("location");
    if (!tokenizer.hasMore() || tokenizer.peek() == "{")
        throw std::runtime_error("Location must have a path (e.g., location / { ... })");
    location.path = tokenizer.consume();
    if (location.path.empty())
        throw std::runtime_error("Location path cannot be empty");
    if (location.path[0] != '/')
        throw std::runtime_error("Location path must start with '/', got: " + location.path);
    
    parseBlock(location.ctx, location);
    
    server.locations.push_back(location);
}

bool ConfigParser::parseLocDirective(const std::string& directive, LocationConfig& location)
{
    if (directive == "methods" || directive == "cgi_ext")
        parseString(directive, (directive == "methods" ? &location.allowedMethods : &location.cgiExtensions));
    else if (directive == "return")
        parseReturn(location);
    else if (directive == "upload" || directive == "cgi")
        parseBool(directive, (directive == "upload" ? location.uploadEnabled : location.cgiEnabled));
    else if (directive == "upload_path")
        parseString("upload_path", NULL, &location.uploadPath);
    else
        return false;
    return true;
}

void ConfigParser::parseBodySize(ConfigContext& ctx)
{
    tokenizer.consume();
    std::string val = getValue("client_max_body_size");
    long long size = parseSize(val);
    std::ostringstream oss;
    
    oss << size;
    ctx.clientMaxBodySize = oss.str();
    tokenizer.expect(";");
}

void ConfigParser::parseErrorPage(ConfigContext& ctx)
{
    tokenizer.consume();
    if (!tokenizer.hasMore())
        throw std::runtime_error("error_page directive requires status code(s) and path");
    std::vector<int> codes;
    
    while (tokenizer.hasMore() && tokenizer.peek() != ";")
    {
        std::string tok = tokenizer.peek();
        
        if (tok.find_first_not_of("0123456789") == std::string::npos)
        {
            int code = std::atoi(tok.c_str());
            if (!validator.isValidStatus(code))
                throw std::runtime_error("Invalid HTTP status code: " + tok);
            if (!ConfigValidator::isDuplicate(codes, code))
                codes.push_back(code);
            tokenizer.consume();
        }
        else
            break;
    }
    if (codes.empty())
        throw std::runtime_error("error_page directive requires at least one status code");
    if (!tokenizer.hasMore() || tokenizer.peek() == ";")
        throw std::runtime_error("error_page directive requires a path");
    std::string path = tokenizer.consume();
    
    for (size_t i = 0; i < codes.size(); i++)
        ctx.errorPages[codes[i]] = path;
    tokenizer.expect(";");
}

void ConfigParser::parseListen(ServerConfig& server)
{
    tokenizer.consume();
    if (!tokenizer.hasMore())
        throw std::runtime_error("listen directive requires address:port or port");
    ListenConfig conf = parseListen(tokenizer.consumeValue()); 
    
    if (server.listen.size() == 1 && 
        server.listen[0].host == "0.0.0.0" && 
        server.listen[0].port == 8080)
        server.listen.clear();
    if (!ConfigValidator::isDuplicate(server.listen, conf))
        server.listen.push_back(conf);
    tokenizer.expect(";");
}

void ConfigParser::parseReturn(LocationConfig& location)
{
    tokenizer.consume();
    if (!tokenizer.hasMore())
        throw std::runtime_error("return directive requires status code and URL");
    std::string codeStr = tokenizer.consumeValue();  
    
    location.redirectCode = std::atoi(codeStr.c_str());
    
    if (!validator.isValidRedirectCode(location.redirectCode))
    {
        std::ostringstream oss;
        oss << "return directive only supports redirect codes (300, 301, 302), got: " << location.redirectCode;
        throw std::runtime_error(oss.str());
    }
    
    if (!tokenizer.hasMore() || tokenizer.peek() == ";")
        throw std::runtime_error("return directive requires a URL/path for redirect");
    
    location.redirectUrl = tokenizer.consumeValue();  
    tokenizer.expect(";");
}

void ConfigParser::parseString(const std::string& directive, std::vector<std::string>* target, std::string *single)
{
    tokenizer.consume();
    ConfigValidator::checkValue(tokenizer, directive);
    if (target == NULL)
        *single = tokenizer.consumeValue();
    else
    {
        target->clear();
        while (tokenizer.hasMore() && tokenizer.peek() != ";")
        {
            std::string next = tokenizer.peek();
            if (next == "}" || next == "{" || ConfigValidator::isReserved(next))
                break;
            std::string val = getValue(directive);
            
            if (!ConfigValidator::isDuplicate(*target, val))
                target->push_back(val);
        }
    }
    tokenizer.expect(";");
}

void ConfigParser::parseBool(const std::string& directive, int& target)
{
    tokenizer.consume();
    std::string val = getValue(directive);
    if (val == "on")
        target = 1;
    else if (val == "off")
        target = 0;
    else
        throw std::runtime_error("Invalid value for " + directive + ": " + val + ". Expected 'on' or 'off'.");
    tokenizer.expect(";");
}

int ConfigParser::parsePort(const std::string& port) const
{
    if (!ConfigValidator::isValidPort(port))
        throw std::runtime_error("Invalid port: " + port);
    return std::atoi(port.c_str());
}

long long ConfigParser::parseSize(const std::string& val) const
{
    if (val.empty())
        throw std::runtime_error("Size value cannot be empty");
    if (val == "0")
        return 0;
    char last = val[val.length() - 1];
    long long mult = 1;
    std::string num;
    
    if (last == 'k' || last == 'K')
        mult = 1024;
    else if (last == 'm' || last == 'M')
        mult = 1024 * 1024;
    else if (last == 'g' || last == 'G')
        mult = 1024 * 1024 * 1024;    
    else
        num = val;
    if (num.empty())
        num = val.substr(0, val.length() - 1);
    if (!ConfigValidator::isDigits(num))
        throw std::runtime_error("Invalid size value: " + val);
    long long n = std::atoll(num.c_str());
    if (n < 0)
        throw std::runtime_error("Invalid size value: " + val);
    return n * mult;
}

ListenConfig ConfigParser::parseListen(const std::string& val) const
{
    ListenConfig conf;
    size_t colon = val.find(':');
    
    if (colon != std::string::npos)
    {
        conf.host = val.substr(0, colon);
        std::string port = val.substr(colon + 1);
        
        if (!ConfigValidator::isValidIP(conf.host) && !ConfigValidator::isValidHostname(conf.host))
            throw std::runtime_error("Invalid IP address or hostname: " + conf.host);
        conf.port = parsePort(port);
    }
    else if (ConfigValidator::isValidPort(val))
    {
        conf.host = "0.0.0.0";
        conf.port = parsePort(val);
    }
    else if (ConfigValidator::isValidIP(val) || ConfigValidator::isValidHostname(val))
    {
        conf.host = val;
        conf.port = 8080;
    }
    else
        throw std::runtime_error("Invalid listen directive: " + val);
    return conf;
}

std::string ConfigParser::getValue(const std::string& directive)
{
    ConfigValidator::checkValue(tokenizer, directive);
    std::string val = tokenizer.consumeValue();
    return val;
}

const HttpConfig& ConfigParser::getConfig() const
{
    return config;
}

void ConfigParser::applyDefaults()
{
    config.ctx.applyDefaults();
    
    for (size_t i = 0; i < config.servers.size(); i++)
    {
        config.servers[i].ctx.inheritFrom(config.ctx);
        config.servers[i].applyDefaults();
    }
}