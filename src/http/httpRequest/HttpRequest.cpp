/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:29:27 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 21:19:20 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpRequestValidator.hpp" 
#include "HttpRequestHandler.hpp"   
#include "HttpRequestParser.hpp"

HttpRequest::HttpRequest(
    const HttpConfig& config,
    const std::string& reqStr,
    const std::string& localIp,
    int localPort)
    : httpConfig(config),
      server(NULL),
      location(NULL),
      _localIp(localIp),
      _localPort(localPort),
      request(reqStr),
      redirectCode(0)
{
    processRawRequest();
}

HttpRequest::HttpRequest(const HttpRequest& other)
    : httpConfig(other.httpConfig), 
      server(other.server),
      location(other.location),
      _localIp(other._localIp),
      _localPort(other._localPort),
      request(other.request),
      method(other.method),
      uri(other.uri),
      httpVersion(other.httpVersion),
      headers(other.headers),
      body(other.body),
      status(other.status),
      finalPath(other.finalPath),
      redirectUri(other.redirectUri),
      redirectCode(other.redirectCode)
{}

void HttpRequest::processRawRequest()
{
    status = parseRequest();
    if (status != REQ_OK)
        return;

    status = validateRequest();
    if (status != REQ_OK)
        return;

    status = handleRequest();
    return;
}

RequestStatus HttpRequest::parseRequest()
{
    HttpRequestParser parser(request);
    RequestStatus status = parser.parse();
    if (status != REQ_OK)
        return status;

    method = parser.getMethod();
    uri = parser.getUri();
    httpVersion = parser.getHttpVersion();
    headers = parser.getHeaders();
    body = parser.getBody();

    return REQ_OK;
}

RequestStatus HttpRequest::validateRequest()
{
    HttpRequestValidator validator(*this);
    RequestStatus status = validator.validate();
    if (status != REQ_OK)
        return status;

    server = validator.getServer();
    location = validator.getLocation();
    body = validator.getBody();
    return REQ_OK;
}

RequestStatus HttpRequest::handleRequest()
{
    HttpRequestHandler handler(*this);
    RequestStatus status = handler.handleRequest();
    finalPath = handler.getFinalPath();
    if (status >= 400)
    {
        handler.setErrorPagePath();
        finalPath = handler.getFinalPath();
    }
    return status;
}

const std::string& HttpRequest::getMethod() const
{
    return method;
}

const std::string& HttpRequest::getUri() const
{
    return uri;
}

const std::string& HttpRequest::getHttpVersion() const
{
    return httpVersion;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const
{
    return headers;
}

const std::string& HttpRequest::getBody() const
{
    return body;
}

short HttpRequest::getRedirectCode() const
{
    return redirectCode;
}

const std::string& HttpRequest::getRedirectUri() const
{
    return redirectUri;
}

const RequestStatus &HttpRequest::getStatus() const
{
    return status;
}

const LocationConfig* HttpRequest::getLocation() const
{
    return location;
}

const ServerConfig* HttpRequest::getServer() const
{
    return server;
}

const std::string& HttpRequest::getFinalPath() const
{
    return finalPath;
}

const HttpConfig& HttpRequest::getHttpConfig() const
{
    return httpConfig;
}

const std::string HttpRequest::getLocalIp() const
{
    return _localIp;
}

int HttpRequest::getLocalPort() const
{
    return _localPort;
}
