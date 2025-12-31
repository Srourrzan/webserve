/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 23:31:33 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 21:00:44 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ConfigValidator.hpp"
#include "RequestStatus.hpp"
#include <string>
#include <map>

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

        RequestStatus parseRequest();
        RequestStatus validateRequest();
        RequestStatus handleRequest();
};

