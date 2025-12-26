/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:29:27 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/27 01:10:22 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const HttpConfig& config,
    const std::string& reqStr,
    const std::string& localIp,
    int localPort)
    : httpConfig(config), request(reqStr),
    _localIp(localIp), _localPort(localPort), redirectCode(0)
{
    parseRequest();
}


RequestStatus HttpRequest::parseRequest()
{
    std::istringstream requestStream(request);
    std::string line;
    
    if (!std::getline(requestStream, line))
        return REQ_BAD_REQUEST;
    if (!line.empty() && line[line.length() - 1] == '\r')
        line = line.substr(0, line.length() - 1);
    std::istringstream requestLineStream(line);
    requestLineStream >> method >> uri >> httpVersion;
    while (std::getline(requestStream, line) && line != "\r")
    {
        if (!line.empty() && line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);
        size_t colonPos = line.find(": ");
        if (colonPos == std::string::npos)
            return REQ_BAD_REQUEST;
        std::string headerKey = line.substr(0, colonPos);
        std::string headerValue = line.substr(colonPos + 2);
        headers[headerKey] = headerValue;
    }
    std::ostringstream bodyStream;
    bodyStream << requestStream.rdbuf();
    body = bodyStream.str();
    return REQ_OK;
}

RequestStatus HttpRequest::isValidRequestLine()
{
    if (method.empty() || uri.empty() || httpVersion.empty())
        return REQ_BAD_REQUEST;
    if (method != "GET" && method != "POST" && method != "DELETE")
        return REQ_METHOD_NOT_ALLOWED;
    if (httpVersion != "HTTP/1.1")
        return REQ_VERSION_NOT_SUPPORTED;
    return REQ_OK;
}

RequestStatus HttpRequest::isValidHeader()
{
    const std::string host = headers.count("Host") ? headers.at("Host") : "";
    ServerConfig *server = NULL;
    const LocationConfig *location = NULL;

    if (host.empty())
        return REQ_BAD_REQUEST;
    server = httpConfig.findServerByHost(host, _localIp, _localPort);
    if (!server)
        return REQ_BAD_REQUEST;
    location = &(server->findLocationByUri(uri));
    if (!location)
        return REQ_BAD_REQUEST;
    if (location->redirectCode != 0)
    {
        redirectCode = location->redirectCode;
        redirectUri = location->redirectUrl;
        if (redirectCode == 300)
            return REQ_MULTIPLE_CHOICES;
        if (redirectCode == 301)
            return REQ_MOVED_PERMANENTLY;
        if (redirectCode == 302)
            return REQ_FOUND;
    }
    if (std::find(location->allowedMethods.begin(), location->allowedMethods.end(), method) == location->allowedMethods.end())
        return REQ_METHOD_NOT_ALLOWED;
    //check folder and files and body
    return REQ_OK;
}

RequestStatus HttpRequest::isValidRequestBody()
{
    if (method == "POST" && body.empty())
        return REQ_BAD_REQUEST;
    //body validation added here
    return REQ_OK;
}

RequestStatus HttpRequest::isValidRequest()
{
    RequestStatus status;
    
    status = isValidRequestLine();
    if (status != REQ_OK)
        return status;
    status = isValidHeader();
    if (status != REQ_OK)
        return status;
    status = isValidRequestBody();
    if (status != REQ_OK)
        return status;
    return REQ_OK;
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


