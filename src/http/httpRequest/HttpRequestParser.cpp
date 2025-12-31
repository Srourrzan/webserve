/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 16:55:08 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 20:18:55 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestParser.hpp"

#include "HttpRequestParser.hpp"

HttpRequestParser::HttpRequestParser(const std::string& rawRequest)
    : stream(rawRequest) {}

RequestStatus HttpRequestParser::parse()
{
    RequestStatus status;

    status = parseRequestLine();
    if (status != REQ_OK)
        return status;

    status = parseHeaders();
    if (status != REQ_OK)
        return status;

    return parseBody();
}

RequestStatus HttpRequestParser::parseRequestLine()
{
    std::string line;
    if (!std::getline(stream, line))
        return REQ_BAD_REQUEST;
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    std::istringstream lineStream(line);
    if (!(lineStream >> method >> uri >> httpVersion))
        return REQ_BAD_REQUEST;

    return REQ_OK;
}

RequestStatus HttpRequestParser::parseHeaders()
{
    std::string line;

    while (std::getline(stream, line))
    {
        if (line == "\r" || line.empty())
            break;
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        size_t colonPos = line.find(": ");
        if (colonPos == std::string::npos)
            return REQ_BAD_REQUEST;

        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 2);
        headers[key] = value;
    }
    return REQ_OK;
}

RequestStatus HttpRequestParser::parseBody()
{
    std::ostringstream bodyStream;
    bodyStream << stream.rdbuf();
    body = bodyStream.str();
    stripCRLF(body);
    return REQ_OK;
}

const std::string& HttpRequestParser::getMethod() const
{
    return method;
}

const std::string& HttpRequestParser::getUri() const
{
    return uri;
}

const std::string& HttpRequestParser::getHttpVersion() const
{
    return httpVersion;
}

const std::map<std::string, std::string>& HttpRequestParser::getHeaders() const
{
    return headers;
}

const std::string& HttpRequestParser::getBody() const
{
    return body;
}

