/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:29:27 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/07 16:04:30 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "HttpRequestValidator.hpp"
#include "HttpRequestHandler.hpp"
#include "HttpRequestParser.hpp"

HttpRequest::HttpRequest(
		const HttpConfig &config,
		const std::string &reqStr,
		const std::string &localIp,
		int localPort)
		: httpConfig(config),
			server(NULL),
			location(NULL),
			_localIp(localIp),
			_localPort(localPort),
			request(reqStr),
			redirectCode(0), 
			isCgi(false)
// addcgi

{

	processRawRequest();
}

HttpRequest::HttpRequest(const HttpRequest &other)
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
// add cgi
{
}

Cgi &HttpRequest::getCgi()
{
	return cgi;
}
void HttpRequest::processRawRequest()
{
	parseRequest();
	if (status != REQ_OK)
		return;

	validateRequest();
	if (status != REQ_OK)
		return;

	handleRequest();
	// state = READING;
	return;
}

void HttpRequest::parseRequest()
{
	HttpRequestParser parser(request);
	status = parser.parse();
	method = parser.getMethod();
	uri = parser.getUri();
	httpVersion = parser.getHttpVersion();
	headers = parser.getHeaders();
	body = parser.getBody();
	if (status != REQ_OK)
		handleErrorPageIfNeeded();
}

void HttpRequest::validateRequest()
{
	HttpRequestValidator validator(*this);
	status = validator.validate();
	server = validator.getServer();
	location = validator.getLocation();
	body = validator.getBody();
	if (status >= 300 && status < 400)
	{
		redirectCode = validator.getRedirectCode();
		redirectUri = validator.getRedirectUri();
	}
	if (status != REQ_OK)
		handleErrorPageIfNeeded();
}

void HttpRequest::handleRequest()
{
	HttpRequestHandler handler(*this);
	status = handler.handleRequest();
	finalPath = handler.getFinalPath();
	isCgi = handler.isCgi;
	if (status != REQ_OK)
		handleErrorPageIfNeeded();
}

void HttpRequest::handleErrorPageIfNeeded()
{
	HttpRequestHandler handler(*this);

	if (status >= 400)
	{
		handler.setErrorPagePath();
		finalPath = handler.getFinalPath();
	}
}

const std::string &HttpRequest::getMethod() const
{
	return method;
}

const std::string &HttpRequest::getUri() const
{
	return uri;
}

const std::string &HttpRequest::getHttpVersion() const
{
	return httpVersion;
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return headers;
}

const std::string &HttpRequest::getBody() const
{
	return body;
}

short HttpRequest::getRedirectCode() const
{
	return redirectCode;
}

const std::string &HttpRequest::getRedirectUri() const
{
	return redirectUri;
}

const RequestStatus &HttpRequest::getStatus() const
{
	return status;
}

const LocationConfig *HttpRequest::getLocation() const
{
	return location;
}

const ServerConfig *HttpRequest::getServer() const
{
	return server;
}

const std::string &HttpRequest::getFinalPath() const
{
	return finalPath;
}

const HttpConfig &HttpRequest::getHttpConfig() const
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

std::ostream &operator<<(std::ostream &out, const HttpRequest &data)
{
	out << "HttpRequest Info: "
			<< "\nStatus: "
			<< data.getStatus()
			<< "\nMethod: "
			<< data.getMethod()
			<< "\nURI: "
			<< data.getUri()
			<< "\nHTTP Version: "
			<< data.getHttpVersion()
			<< "\nlocation\n"
			<< data.getLocation()
			<< "\nRedirect Code: "
			<< data.getRedirectCode()
			<< "\nRedirect URI: "
			<< data.getRedirectUri()
			<< "\nFinal Path: "
			<< data.getFinalPath()
			<< "\nHeaders: ";
	for (std::map<std::string, std::string>::const_iterator it = data.getHeaders().begin(); it != data.getHeaders().end(); ++it)
		out << "  " << it->first << ": " << it->second << "\n";
	out << "Body: [" << data.getBody() << "]\n";
	return (out);
}

void HttpRequest::setStatus(RequestStatus newStatus)
{
	this->status = newStatus;
}

void HttpRequest::setFinalPath(std::string &path)
{
	this->finalPath = path;
}