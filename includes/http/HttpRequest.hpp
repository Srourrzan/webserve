/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:14:23 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/27 01:17:01 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "ConfigValidator.hpp"

enum RequestStatus
{
    REQ_OK = 200,
    REQ_MULTIPLE_CHOICES = 300,
    REQ_MOVED_PERMANENTLY = 301,
    REQ_FOUND = 302,
    REQ_BAD_REQUEST = 400,
    REQ_NOT_FOUND = 404,
    REQ_METHOD_NOT_ALLOWED = 405,
    REQ_LENGTH_REQUIRED = 411,
    REQ_PAYLOAD_TOO_LARGE = 413,
    REQ_URI_TOO_LONG = 414,
    REQ_NOT_IMPLEMENTED = 501,
    REQ_VERSION_NOT_SUPPORTED = 505
};

class HttpRequest
{
    private:
        const HttpConfig httpConfig;
        const std::string request;
        std::string _localIp;
        int _localPort;
        
        short redirectCode;
        std::string redirectUri;

        std::string method;
        std::string uri;
        std::string httpVersion;
        std::map<std::string, std::string> headers;
        std::string body;
        
    public:
        HttpRequest(const HttpConfig& config, const std::string& reqStr,
                    const std::string& localIp, int localPort);
        RequestStatus parseRequest();
        RequestStatus isValidRequest();
        RequestStatus isValidRequestLine();
        RequestStatus isValidHeader();
        RequestStatus isValidRequestBody();
        const std::string& getMethod() const;
        const std::string& getUri() const;
        const std::string& getHttpVersion() const;
        const std::map<std::string, std::string>& getHeaders() const;
        const std::string& getBody() const;
        short getRedirectCode() const;
        const std::string& getRedirectUri() const;
};

#endif