/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 23:31:33 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/07 16:04:37 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ConfigValidator.hpp"
#include "RequestStatus.hpp"
#include <string>
#include <map>
#include"Cgi.hpp"

class HttpRequest
{
public:
    HttpRequest(const HttpConfig& config, const std::string& reqStr,
                const std::string& localIp, int localPort);
    HttpRequest(const HttpRequest& other);

    const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    const std::string& getFinalPath() const;
    short getRedirectCode() const;
    const std::string& getRedirectUri() const;
    const RequestStatus& getStatus() const;
    const HttpConfig& getHttpConfig() const;
    const ServerConfig* getServer() const;
    const LocationConfig* getLocation() const;
    const std::string getLocalIp() const;
    int getLocalPort() const ;
    void processRawRequest();
    Cgi& getCgi();
    bool cgiFlag;
    void setStatus(RequestStatus newStatus);
    void setFinalPath(std::string &path);


private:
    const HttpConfig httpConfig;
    const ServerConfig* server;
    const LocationConfig* location;
    const std::string _localIp;
    const int _localPort;
    const std::string request;
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;
    RequestStatus status;
    std::string finalPath;
    std::string redirectUri;
    short redirectCode;

    void parseRequest();
    void validateRequest();
    void handleRequest();
    void handleErrorPageIfNeeded();
    Cgi cgi;

};

std::ostream& operator<< (std::ostream &out, const HttpRequest& data);