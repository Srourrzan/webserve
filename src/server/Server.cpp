/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:04 by dikhalil          #+#    #+#             */
/*   Updated: 2026/01/31 20:10:03 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

// void SetStatus(State& currentState, State newState)
// {
// 	currentState = newState;
// }

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
	if (headerEnd == std::string::npos)
		return false;
	std::string header = buffer.substr(0, headerEnd);
	std::string body = buffer.substr(headerEnd + 4);

	if (header.find("GET ") == 0 || header.find("DELETE ") == 0)
		return true;
	if (header.find("POST ") == 0)
	{
		size_t pos = header.find("Content-Length: ");
		if (pos != std::string::npos)
		{
			unsigned long clientLen = strToUL(header.substr(pos + 15));
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

void Server::readFromClient(Socket &client)
{
	HttpResponse response;
	std::string localIp;
	int localPort = 0;
	char buffer[BUFFER_SIZE];
	std::memset(buffer, 0, BUFFER_SIZE);
	ssize_t bytesRead = recv(client.fd, buffer, BUFFER_SIZE - 1, 0);

	if (bytesRead <= 0)
	{
		closeSocket(this->_clientSockets, client.fd);
		return;
	}
	client.lastActivity = std::time(NULL);
	if (client.buffer.size() + bytesRead > 1048576)
	{
		std::cerr << "Server Error: Request too large from " << client.host << std::endl;
		closeSocket(this->_clientSockets, client.fd);
		return;
	}
	client.buffer.append(buffer, bytesRead);
	if (requestIsComplete(client.buffer))
	{
		// std::cout << "\n========== Received Request ==========\n\n";
		// std::cout << client.buffer;
		// std::cout << "======================================\n";
		Socket *ls = findSocket(this->_listenSockets, client.listenFd);
		if (ls)
		{
			localIp = ls->host;
			localPort = ls->port;
		}
		HttpRequest request(_config, client.buffer, localIp, localPort);

		if (request.isCgi)
		{
			if (!request.getCgi().isStdoutClosed())
				addToPoll(request.getCgi().getStdoutFd(), POLLIN);

			if (!request.getCgi().isStdinClosed() && request.getMethod() == "POST")
				addToPoll(request.getCgi().getStdinFd(), POLLOUT);
		}
		client.request = request; // Store the request in the client socket for later use
		std::cout << "\n===== HttpRequest Info =====\n"
				  << std::endl;
		std::cout << request;
		std::cout << "\n============================\n"
				  << std::endl;

	
		
		response.buildResponse(request);
		std::cout << response;
		client.responseString = response.getFullResponse();
		changePollEvent(client.fd, POLLOUT);
		// SetStatus(request.state, SENDING);
	}
}

void Server::writeToClient(Socket &client)
{

	// std::cout << "Writing response to client fd: " << client.fd << std::endl;
	std::string &response = client.responseString;
	// std::cout << "Response size: " << response.size() << ", Already sent: " << client.totalSent << std::endl;
	// std::cout << "Attempting to send " << (response.size() - client.totalSent) << " bytes..." << std::endl;
	ssize_t sent = send(client.fd, response.c_str() + client.totalSent, response.size() - client.totalSent, 0);
	// std::cout << "send() returned: " << sent << std::endl;
	if (sent <= 0)
	{
		std::cerr << "Error: send failed with result: " << sent << ", errno: " << errno << std::endl;
		closeSocket(this->_clientSockets, client.fd);
		return;
	}
	// std::cout << "Sent " << sent << " bytes to client" << std::endl;
	client.lastActivity = std::time(NULL);
	client.totalSent += sent;
	if (client.totalSent >= response.size())
	{
		// std::cout << "Response complete, switching back to POLLIN" << std::endl;
		client.totalSent = 0;
		client.buffer.clear();
		changePollEvent(client.fd, POLLIN);
		// if (request.getHeaders().count("Connection") && request.getHeaders().at("Connection") == "close")
		//         closeSocket(clientSockets, client.fd);
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
	
    HttpRequest &req = client->request;

	if (req.isCgi)
	{
		if (req.getCgi().getStdoutFd() == fd && this->_pollFds[index].revents & POLLIN)
		{
			req.getCgi().handleCgiOutput(req);
			if (req.getCgi().isStdoutClosed())
                removePollFd(fd);
			return ;
		}
		 if (req.getCgi().getStdinFd() == fd && (this->_pollFds[index].revents & POLLOUT))
        {
            req.getCgi().handleCgiBody(req); 
            if (req.getCgi().isStdinClosed())
                removePollFd(fd);
            return;
        }
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
//inside server map fd - req
//add fd 
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