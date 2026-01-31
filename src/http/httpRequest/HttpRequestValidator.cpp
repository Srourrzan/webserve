/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestValidator.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 17:06:52 by dikhalil          #+#    #+#             */
/*   Updated: 2026/01/31 20:51:09 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestValidator.hpp"

HttpRequestValidator::HttpRequestValidator(const HttpRequest& req)
    : req(req), server(NULL), location(NULL), redirectCode(0), body(req.getBody()) { }

RequestStatus HttpRequestValidator::validate()
{
    RequestStatus status;

    status = validateRequestLine();
    if (status != REQ_OK)
        return status;

    status = validateHeaders();
    if (status != REQ_OK)
        return status;

    status = validateBody();
    if (status != REQ_OK)
        return status;

    return REQ_OK;
}

RequestStatus HttpRequestValidator::validateRequestLine()
{
    const std::string& method = req.getMethod();
    const std::string& uri = req.getUri();
    const std::string& version = req.getHttpVersion();

    if (method.empty() || uri.empty() || version.empty())
        return REQ_BAD_REQUEST;
    if (method != "GET" && method != "POST" && method != "DELETE")
        return REQ_METHOD_NOT_ALLOWED;
    if (uri.length() > 2048)
        return REQ_URI_TOO_LONG;
    if (version != "HTTP/1.1")
        return REQ_VERSION_NOT_SUPPORTED;
    return REQ_OK;
}

RequestStatus HttpRequestValidator::validateHeaders()
{
    RequestStatus status;

    status = validateHost();
    if (status != REQ_OK)
        return status;

    status = checkRedirection();
    if (status != REQ_OK)
        return status;

    status = checkAllowedMethods();
    if (status != REQ_OK)
        return status;

    return REQ_OK;
}

RequestStatus HttpRequestValidator::validateHost()
{
    const std::map<std::string,std::string>& headers = req.getHeaders();
    if (!headers.count("Host"))
        return REQ_BAD_REQUEST;
    server = req.getHttpConfig().findServerByHost(headers.at("Host"), req.getLocalIp(), req.getLocalPort());
    if (!server)
        return REQ_BAD_REQUEST;
    location = server->findLocationByUri(req.getUri());
    if (!location)
        return REQ_NOT_FOUND;
    return REQ_OK;
}

RequestStatus HttpRequestValidator::checkRedirection()
{
    if (location->redirectCode == 0)
        return REQ_OK;

    redirectCode = location->redirectCode;
    redirectUri = location->redirectUrl;

    if (redirectCode == 300)
        return REQ_MULTIPLE_CHOICES;
    if (redirectCode == 301)
        return REQ_MOVED_PERMANENTLY;
    if (redirectCode == 302)
        return REQ_FOUND;
    return REQ_OK;
}

RequestStatus HttpRequestValidator::checkAllowedMethods()
{
    const std::vector<std::string>& allowed = location->allowedMethods;
    if (std::find(allowed.begin(), allowed.end(),
        req.getMethod()) == allowed.end())
        return REQ_METHOD_NOT_ALLOWED;
    return REQ_OK;
}

RequestStatus HttpRequestValidator::validateBody()
{
    if (req.getMethod() != "POST")
        return REQ_OK;
    if (body.empty())
        return REQ_BAD_REQUEST;

    RequestStatus status = validateChunkedEncoding();
    if (status != REQ_OK)
        return status;
        
    size_t maxSize = strToUL(req.getLocation()->ctx.clientMaxBodySize);
    if (maxSize && body.size() > maxSize)
        return REQ_PAYLOAD_TOO_LARGE;
    return REQ_OK;
}

RequestStatus HttpRequestValidator::validateContentLength()
{
	const std::map<std::string,std::string>& headers = req.getHeaders();
	if (!headers.count("Content-Length"))
		return REQ_NOT_IMPLEMENTED;
	size_t contentLength = strToUL(headers.at("Content-Length"));
	if (body.size() != contentLength)
		return REQ_BAD_REQUEST;
	return REQ_OK;
}

RequestStatus HttpRequestValidator::validateChunkedEncoding()
{
	const std::map<std::string,std::string>& headers = req.getHeaders();
	if (!headers.count("Transfer-Encoding"))
		return validateContentLength();
	if (headers.at("Transfer-Encoding") != "chunked")
		return REQ_NOT_IMPLEMENTED;
	if (!unchunkBody(body))
		return REQ_BAD_REQUEST;
	return REQ_OK;
}

bool HttpRequestValidator::unchunkBody(std::string &body)
{
	std::istringstream stream(body);
	std::ostringstream unchunked;
	std::string line;

	while (std::getline(stream, line))
	{
		if (line == "0")
			break;
		size_t pos = line.find(';');
		if (pos != std::string::npos) line = line.substr(0, pos);
		size_t chunkSize;
		std::istringstream hex(line);
		hex >> std::hex >> chunkSize;
		if (hex.fail())
			return false;
		std::vector<char> buffer(chunkSize);
		stream.read(&buffer[0], chunkSize);
		if (stream.gcount() != (std::streamsize)chunkSize)
			return false;
		unchunked.write(&buffer[0], chunkSize);
		if (unchunked.fail())
			return false;
		std::getline(stream, line);
	}
	body = unchunked.str();
	return true;
}

const std::string& HttpRequestValidator::getBody() const
{
    return body;
}

const ServerConfig* HttpRequestValidator::getServer() const
{
    return server;
}

const LocationConfig* HttpRequestValidator::getLocation() const
{
    return location;
}

short HttpRequestValidator::getRedirectCode() const
{
    return redirectCode;
}

const std::string& HttpRequestValidator::getRedirectUri() const
{
    return redirectUri;
}