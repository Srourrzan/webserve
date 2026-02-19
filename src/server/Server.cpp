/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:04 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/19 23:51:11 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <sys/wait.h>


Server::Server(const HttpConfig &config) : _config(config)
{
	this->_listenSockets.clear();
	this->_clientSockets.clear();
	fillListenSockets(_config);
	initListenSockets();

	std::cout << "===========================\n";
	std::cout << "\nServer started successfully!\n";
	std::cout << *this;
	std::cout << "\n===========================\n";
}

Server::~Server()
{
	closeAllSockets(this->_clientSockets);
	closeAllSockets(this->_listenSockets);
}

void Server::closeAllSockets(std::vector<Socket> &sockets)
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		if (sockets[i].request != NULL)
		{
			delete sockets[i].request;
			sockets[i].request = NULL;
		}
		if (sockets[i].fd != -1)
			close(sockets[i].fd);
	}
	sockets.clear();
}

void Server::closeSocket(std::vector<Socket> &sockets, int fd)
{
	for (size_t i = 0; i < this->_pollFds.size(); i++)
	{
		if (this->_pollFds[i].fd == fd)
		{
			this->_pollFds.erase(this->_pollFds.begin() + i);
			break;
		}
	}
	for (size_t i = 0; i < sockets.size(); i++)
	{
		if (sockets[i].fd == fd)
		{
			if (sockets[i].request != NULL)
			{
				delete sockets[i].request;
				sockets[i].request = NULL;
			}
			close(fd);
			sockets.erase(sockets.begin() + i);
			return;
		}
	}
}

bool Server::isDuplicateSocket(const std::string &host, int port) const
{
	for (size_t i = 0; i < this->_listenSockets.size(); i++)
	{
		if (this->_listenSockets[i].host == host && this->_listenSockets[i].port == port)
		{
			return true;
		}
	}
	return false;
}

void Server::fillListenSockets(const HttpConfig &config)
{
	for (size_t i = 0; i < config.servers.size(); i++)
	{
		const ServerConfig &srv = config.servers[i];
		for (size_t j = 0; j < srv.listen.size(); j++)
		{
			if (!isDuplicateSocket(srv.listen[j].host, srv.listen[j].port))
			{
				Socket ls;
				ls.host = srv.listen[j].host;
				ls.port = srv.listen[j].port;
				ls.fd = -1;
				this->_listenSockets.push_back(ls);
			}
		}
	}
}

bool Server::closeSocketOnError(Socket &ls, const std::string &errorMsg)
{
	std::cerr << errorMsg << std::endl;
	close(ls.fd);
	ls.fd = -1;
	return false;
}

struct addrinfo *Server::getAddressInfo(const Socket &ls)
{
	struct addrinfo hints, *res = NULL;
	std::ostringstream portStr;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	portStr << ls.port;
	if (getaddrinfo(ls.host.c_str(), portStr.str().c_str(), &hints, &res) != 0)
	{
		std::cerr << "Server Error: getaddrinfo failed for " + ls.host + ":" + portStr.str() << std::endl;
		return NULL;
	}
	return res;
}

bool Server::setupSocket(Socket &ls, struct addrinfo *addr)
{
	std::ostringstream portStr;
	portStr << ls.port;
	std::string address = ls.host + ":" + portStr.str();
	ls.fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (ls.fd == -1)
		return false;
	setNonBlocking(ls.fd);
	int opt = 1;
	if (setsockopt(ls.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		return closeSocketOnError(ls, "setsockopt failed for " + address);
	if (bind(ls.fd, addr->ai_addr, addr->ai_addrlen) == -1)
		return closeSocketOnError(ls, "warning: could not bind to ..." + address);
	if (listen(ls.fd, SOMAXCONN) == -1)
		return closeSocketOnError(ls, "listen failed for " + address);
	return true;
}

void Server::initListenSockets()
{
	std::vector<Socket> successfulSockets;
	for (size_t i = 0; i < this->_listenSockets.size(); i++)
	{
		Socket &ls = this->_listenSockets[i];
		struct addrinfo *res = getAddressInfo(ls);
		if (!res)
			continue;
		for (struct addrinfo *p = res; p != NULL; p = p->ai_next)
		{
			Socket newSocket = ls;
			if (setupSocket(newSocket, p))
			{
				successfulSockets.push_back(newSocket);
				break;
			}
		}
		freeaddrinfo(res);
	}
	if (successfulSockets.empty())
		throw std::runtime_error("No valid listening sockets could be created.");
	this->_listenSockets = successfulSockets;
}

void Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl F_GETFL failed");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl F_SETFL failed");
}

void Server::addToPoll(int fd, short events)
{
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	this->_pollFds.push_back(pfd);
}

void Server::initPollFds()
{
	this->_pollFds.clear();
	for (size_t i = 0; i < this->_listenSockets.size(); i++)
		addToPoll(this->_listenSockets[i].fd, POLLIN);
}

void Server::addClientSocket(int clientFd, int listenFd)
{
	Socket cl;
	cl.fd = clientFd;
	cl.listenFd = listenFd;
	cl.lastActivity = std::time(NULL);
	cl.totalSent = 0;
	cl.buffer.clear();
	cl.request = NULL;
	cl.closeAfterResponse = false;
	this->_clientSockets.push_back(cl);
}

void Server::acceptClient(int listenFd)
{
	int clientFd = accept(listenFd, NULL, NULL);

	if (clientFd == -1)
	{
		std::cerr << "Server Error: accept failed" << std::endl;
		return;
	}
	setNonBlocking(clientFd);
	addClientSocket(clientFd, listenFd);
	addToPoll(clientFd, POLLIN);
}

Socket *Server::findSocket(std::vector<Socket> &sockets, int fd)
{
	for (size_t i = 0; i < sockets.size(); i++)
	{
		if (sockets[i].fd == fd)
			return &sockets[i];
	}
	return NULL;
}

bool Server::requestIsComplete(const std::string &buffer)
{
	size_t headerEnd = buffer.find("\r\n\r\n");
	size_t bodyStart = 4;
	
	if (headerEnd == std::string::npos)
	{
		headerEnd = buffer.find("\n\n");
		bodyStart = 2;
		if (headerEnd == std::string::npos)
			return false;
	}
	
	std::string header = buffer.substr(0, headerEnd);
	std::string body = buffer.substr(headerEnd + bodyStart);

	if (header.find("GET ") == 0 || header.find("DELETE ") == 0)
		return true;
	if (header.find("POST ") == 0)
	{
		size_t pos = header.find("Content-Length: ");
		if (pos != std::string::npos)
		{
			unsigned long clientLen = strToUL(header.substr(pos + 16));
			return body.size() >= clientLen;
		}
		if (header.find("Transfer-Encoding: ") != std::string::npos)
			return body.find("0\r\n\r\n") != std::string::npos;
	}
	return false;
}

void Server::changePollEvent(int fd, short events)
{
	for (size_t i = 0; i < this->_pollFds.size(); i++)
	{
		if (this->_pollFds[i].fd == fd)
		{
			this->_pollFds[i].events = events;
			return;
		}
	}
}

void Server::removePollFd(int fd)
{
	for (size_t i = 0; i < this->_pollFds.size(); i++)
	{
		if (this->_pollFds[i].fd == fd)
		{
			this->_pollFds.erase(this->_pollFds.begin() + i);
			return;
		}
	}
}

void Server::handleSocketError(int fd, size_t &index, bool isListen)
{
	std::cerr << "Server Info: Closing " << (isListen ? "listen" : "client")
			  << " socket fd " << fd << " due to error/hangup." << std::endl;
	this->_pollFds.erase(this->_pollFds.begin() + index);
	if (isListen)
	{
		closeSocket(this->_listenSockets, fd);
		if (this->_listenSockets.empty())
			throw std::runtime_error("Server Error: All listening sockets closed.");
	}
	else
		closeSocket(this->_clientSockets, fd);
	index--;
}

void Server::handleListenSocket(size_t &index)
{
	if (this->_pollFds[index].revents & (POLLERR | POLLHUP))
	{
		handleSocketError(this->_pollFds[index].fd, index, true);
		return;
	}
	if (this->_pollFds[index].revents & POLLIN)
		acceptClient(this->_pollFds[index].fd);
}

bool Server::validateRequestLine(const std::string& line)
{
	std::istringstream iss(line);
	std::string method, uri, version;

	if (!(iss >> method >> uri >> version))
		return false;
	if (method != "GET" && method != "POST" && method != "DELETE")
		return false;
	if (version != "HTTP/1.1")
		return false;
	if (uri.empty() || uri[0] != '/')
		return false;
	return true;
}

bool Server::validateHeaders(const std::string& headers)
{
	std::istringstream stream(headers);
	std::string line;
	bool hasHost = false;

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;
		size_t colonPos = line.find(':');
		if (colonPos == std::string::npos)
			return false;
		std::string key = line.substr(0, colonPos);
		std::string value = line.substr(colonPos + 1);
		while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
			value.erase(0, 1);
		if (key.empty())
			return false;
		if (key == "Host" || key == "host")
			hasHost = true;
		if (key == "Content-Length")
		{
			for (size_t i = 0; i < value.size(); i++)
			{
				if (!isdigit(value[i]) && value[i] != ' ' && value[i] != '\t')
					return false;
			}
		}
	}
	if (!hasHost)
		return false;
	return true;
}

bool Server::isMalformedRequest(const std::string& buffer)
{
	std::string headers;
	std::string requestLine;
	std::string lineSep = "\r\n";
	size_t headerEnd = buffer.find("\r\n\r\n");
	
	if (headerEnd == std::string::npos)
	{
		headerEnd = buffer.find("\n\n");
		lineSep = "\n";
		if (headerEnd == std::string::npos)
			return false;
	}
	std::string headerPart = buffer.substr(0, headerEnd);
	size_t firstLineEnd = headerPart.find(lineSep);
	if (firstLineEnd == std::string::npos)
		return true;
	requestLine = headerPart.substr(0, firstLineEnd);
	headers = headerPart.substr(firstLineEnd + lineSep.length());
	if (!validateRequestLine(requestLine))
		return true;
	if (!validateHeaders(headers))
		return true;
	return false;
}

void Server::build400AndClose(Socket &client)
{
	std::ostringstream ss;
	std::string body = "<html><body><h1>400 Bad Request</h1></body></html>";

	ss << "HTTP/1.1 400 Bad Request\r\n";
	ss << "Content-Length: " << body.size() << "\r\n";
	ss << "Content-Type: text/html\r\n";
	ss << "Connection: close\r\n";
	ss << "\r\n";
	ss << body;

	client.responseString = ss.str();
	client.totalSent = 0;
	client.buffer.clear();
	client.closeAfterResponse = true;
	changePollEvent(client.fd, POLLOUT);
}


void Server::readFromClient(Socket &client)
{
	int localPort = 0;
	std::string localIp;
	HttpResponse response;
	char buffer[BUFFER_SIZE];
	std::memset(buffer, 0, BUFFER_SIZE);
	ssize_t bytesRead = recv(client.fd, buffer, BUFFER_SIZE - 1, 0);

	if (bytesRead <= 0)
	{
		closeSocket(this->_clientSockets, client.fd);
		return;
	}
	client.lastActivity = std::time(NULL);
	client.buffer.append(buffer, bytesRead);
	if (isMalformedRequest(client.buffer))
	{
		build400AndClose(client);
		return;
	}
	if (!requestIsComplete(client.buffer))
		return;

	Socket *ls = findSocket(this->_listenSockets, client.listenFd);
	if (ls)
	{
		localIp = ls->host;
		localPort = ls->port;
	}
	client.request = new HttpRequest(_config, client.buffer, localIp, localPort);
	HttpRequest& request = *client.request;
	if (request.getIsCgi())
		response.buildCgiResponse(request);
	else
		response.buildResponse(response, request);
	client.responseString = response.getFullResponse();
	client.totalSent = 0;
	client.closeAfterResponse = false;
	if (request.getHeaders().count("Connection"))
	{
		std::string conn = request.getHeaders().at("Connection");
		if (conn == "close")
			client.closeAfterResponse = true;
	}
	changePollEvent(client.fd, POLLOUT);
	delete(client.request);
}


void Server::writeToClient(Socket &client)
{
	std::string &response = client.responseString;
	ssize_t sent = send(client.fd, response.c_str() + client.totalSent, response.size() - client.totalSent, 0);
	if (sent <= 0)
	{
		std::cerr << "Error: send failed with result: " << sent << ", errno: " << errno << std::endl;
		closeSocket(this->_clientSockets, client.fd);
		return;
	}
	client.lastActivity = std::time(NULL);
	client.totalSent += sent;
	if (client.totalSent >= response.size())
	{
		client.totalSent = 0;
		client.buffer.clear();
		changePollEvent(client.fd, POLLIN);
		if (client.closeAfterResponse)		
		{
			closeSocket(this->_clientSockets, client.fd);
			return;
		}
	}
}

void Server::handleClientSocket(size_t &index)
{
	int fd = this->_pollFds[index].fd;
	Socket *client = findSocket(this->_clientSockets, fd);

	if (!client)
	{
		this->_pollFds.erase(this->_pollFds.begin() + index);
		index--;
		return;
	}
	if (this->_pollFds[index].revents & (POLLERR | POLLHUP))
	{
		handleSocketError(fd, index, false);
		return;
	}
	
	if (this->_pollFds[index].revents & POLLIN)
	{
		readFromClient(*client);
		client = findSocket(this->_clientSockets, fd);
		if (!client)
			return;
	}
	if (this->_pollFds[index].revents & POLLOUT)
	{
		writeToClient(*client);
	}
}

void Server::checkClientTimeouts()
{
	time_t now = time(NULL);
	for (size_t i = 0; i < this->_clientSockets.size(); i++)
	{
		if (now - this->_clientSockets[i].lastActivity > CLIENT_TIMEOUT)
		{
			closeSocket(this->_clientSockets, this->_clientSockets[i].fd);
			i--;
		}
	}
}

void Server::run()
{
	initPollFds();
	while (true)
	{
		if (this->_pollFds.empty())
			throw std::runtime_error("Server Error: No sockets to poll");
		int ret = poll(&this->_pollFds[0], this->_pollFds.size(), POLL_TIMEOUT);
		if (ret == -1)
		{
			closeAllSockets(this->_clientSockets);
			closeAllSockets(this->_listenSockets);
			throw std::runtime_error("Server Error: poll failed");
		}
		if (ret == 0)
		{
			checkClientTimeouts();
			continue;
		}
		for (size_t i = 0; i < this->_pollFds.size(); i++)
		{
			if (this->_pollFds[i].revents == 0)
				continue;
			if (findSocket(this->_listenSockets, this->_pollFds[i].fd) != NULL)
				handleListenSocket(i);
	
			else
				handleClientSocket(i);
		}
	}
}

std::vector<Socket> Server::getListenSockets() const
{
	return (this->_listenSockets);
}

std::ostream &operator<<(std::ostream &out, const Server &data)
{
	std::vector<Socket> listenSockets;

	listenSockets = data.getListenSockets();
	out << "Servet listening on:\n";
	for (size_t i = 0; i < listenSockets.size(); i++)
	{
		out << listenSockets[i].host << ":"
			<< listenSockets[i].port << "\n";
	}
	return (out);
}
