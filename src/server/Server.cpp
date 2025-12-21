/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:04 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/21 02:16:14 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const HttpConfig& config)
{
    listenSockets.clear();
    clientSockets.clear();
    fillListenSockets(config);
    initListenSockets();
    
    std::cout << "===========================\n";
    std::cout << "Server started successfully!\n";
    std::cout << "Listening on:\n";
    for (size_t i = 0; i < listenSockets.size(); i++)
    {
        std::cout << listenSockets[i].host << ":" 
                  << listenSockets[i].port << "\n";
    }
    std::cout << "===========================\n";
}

Server::~Server()
{
    closeAllSockets(clientSockets);
    closeAllSockets(listenSockets);
}

void Server::closeAllSockets(std::vector<Socket>& sockets)
{
    for (size_t i = 0; i < sockets.size(); i++)
    {
        if (sockets[i].fd != -1)
            close(sockets[i].fd);
    }
    sockets.clear();
}

void Server::closeSocket(std::vector<Socket>& sockets, int fd)
{
    for (size_t i = 0; i < pollFds.size(); i++)
    {
        if (pollFds[i].fd == fd)
        {
            pollFds.erase(pollFds.begin() + i);
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

void Server::fillListenSockets(const HttpConfig& config)
{
    for (size_t i = 0; i < config.servers.size(); i++)
    {
        const ServerConfig& srv = config.servers[i];
        for (size_t j = 0; j < srv.listen.size(); j++)
        {
            Socket ls;
            ls.host = srv.listen[j].host;
            ls.port = srv.listen[j].port;
            listenSockets.push_back(ls);
        }
    }
}

bool Server::closeSocketOnError(Socket& ls, const std::string& errorMsg)
{
    std::cerr << "Server Error: " << errorMsg << std::endl;
    close(ls.fd);
    ls.fd = -1;
    return false;
}

struct addrinfo* Server::getAddressInfo(const Socket& ls)
{
    struct addrinfo hints, *res = NULL;
    std::ostringstream portStr;
    
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    portStr << ls.port;
    if (getaddrinfo(ls.host.c_str(), portStr.str().c_str(), &hints, &res) != 0)
    {
        std::cerr << "Server Error: getaddrinfo failed for " + ls.host + ":" + portStr.str() << std::endl;
        return NULL;
    }
    return res;
}

bool Server::setupSocket(Socket& ls, struct addrinfo* addr)
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
        return closeSocketOnError(ls, "bind failed for " + address);
    if (listen(ls.fd, SOMAXCONN) == -1)
        return closeSocketOnError(ls, "listen failed for " + address);
    return true;
}

void Server::initListenSockets()
{
    std::vector<Socket> successfulSockets;
    for (size_t i = 0; i < listenSockets.size(); i++)
    {
        Socket& ls = listenSockets[i];
        struct addrinfo* res = getAddressInfo(ls);
        if (!res)
            continue;
        for (struct addrinfo* p = res; p != NULL; p = p->ai_next)
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
    listenSockets = successfulSockets;
}

void Server::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed");
}

void Server::addToPoll(int fd, short events)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    pollFds.push_back(pfd);
}

void Server::initPollFds()
{
    pollFds.clear();
    for (size_t i = 0; i < listenSockets.size(); i++)
        addToPoll(listenSockets[i].fd, POLLIN);
}

void Server::addClientSocket(int clientFd, struct sockaddr_in& clientAddr)
{
    Socket cl;
    cl.fd = clientFd;
    cl.port = ntohs(clientAddr.sin_port);
    cl.host = inet_ntoa(clientAddr.sin_addr);//forbiden
    clientSockets.push_back(cl);
}

void Server::acceptClient(int listenFd)
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    if (clientFd == -1)
    {
        std::cerr << "Server Error: accept failed" << std::endl;
        return;
    }
    setNonBlocking(clientFd);
    addClientSocket(clientFd, clientAddr);
    addToPoll(clientFd, POLLIN);
}

Socket* Server::findSocket(std::vector<Socket>& sockets, int fd)
{
    for (size_t i = 0; i < sockets.size(); i++)
    {
        if (sockets[i].fd == fd)
            return &sockets[i];
    }
    return NULL;
}

bool Server::requestIsComplete(const std::string& buffer)
{
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;
    std::string header = buffer.substr(0, headerEnd);
    std::string body   = buffer.substr(headerEnd + 4);

    if (header.find("GET ") == 0 || header.find("DELETE ") == 0)
        return true;
    if (header.find("POST ") == 0)
    {
        size_t pos = header.find("Content-Length:");
        if (pos != std::string::npos)
        {
            unsigned long clientLen = strToUL(header.substr(pos + 15));
            return body.size() >= clientLen;
        }
        if (header.find("Transfer-Encoding: chunked") != std::string::npos)
            return body.find("0\r\n\r\n") != std::string::npos;
    }
    return false;
}

void Server::changePollEvent(int fd, short events)
{
    for (size_t i = 0; i < pollFds.size(); i++)
    {
        if (pollFds[i].fd == fd)
        {
            pollFds[i].events = events;
            return;
        }
    }
}

void Server::handleSocketError(int fd, size_t& index, bool isListen)
{
    std::cerr << "Server Info: Closing " << (isListen ? "listen" : "client") 
              << " socket fd " << fd << " due to error/hangup." << std::endl;
    pollFds.erase(pollFds.begin() + index);
    if (isListen)
    {
        closeSocket(listenSockets, fd);
        if (listenSockets.empty())
            throw std::runtime_error("Server Error: All listening sockets closed.");
    }
    else
        closeSocket(clientSockets, fd);
    index--;
}

void Server::handleListenSocket(size_t& index)
{
    if (pollFds[index].revents & (POLLERR | POLLHUP))
    {
        handleSocketError(pollFds[index].fd, index, true);
        return;
    }
    if (pollFds[index].revents & POLLIN)
        acceptClient(pollFds[index].fd);
}

// I should replace the below code to a functional code using HTTP 
void Server::readFromClient(Socket& client)
{
    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE); 
    ssize_t bytesRead = recv(client.fd, buffer, BUFFER_SIZE - 1, 0);  
    
    if (bytesRead == 0)
    {
        std::cerr << "Server Info: Client disconnected, closing fd " << client.fd << std::endl;
        closeSocket(clientSockets, client.fd);
        return;
    }
    if (bytesRead == -1)
    {
        std::cerr << "Server Error: recv failed for client fd " << client.fd << std::endl;
        closeSocket(clientSockets, client.fd);
        return;
    }
    if (client.buffer.size() + bytesRead > 1048576) 
    {
        std::cerr << "Server Error: Request too large from " << client.host << std::endl;
        closeSocket(clientSockets, client.fd);
        return;
    }  
    client.buffer.append(buffer, bytesRead);
    if (requestIsComplete(client.buffer))
    {
        std::cout << "-------------------------------------------\n";
        std::cout << "\nHTTP REQUEST RECEIVED\n";
        std::cout << "Client: " << client.host << ":" << client.port << "\n";
        
        std::istringstream iss(client.buffer);
        std::string line;
        int lineNum = 1;
        while (std::getline(iss, line))
        {
            if (!line.empty() && line[line.size() - 1] == '\r')
                line = line.substr(0, line.size() - 1);
            std::cout << "Line " << lineNum++ << ": " << line << "\n";
        }     
		
        changePollEvent(client.fd, POLLOUT);
    }
}

void Server::writeToClient(Socket& client)
{
    std::string response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 50\r\n"
        "\r\n"
        "<html><body><h1>Hello from webserv!</h1></body></html>";
    
    while (client.totalSent < response.size())
    {
        ssize_t sent = send(client.fd, response.c_str() + client.totalSent, 
                           response.size() - client.totalSent, 0);
        if (sent == -1)
        {
            std::cerr << "Server Error: send failed" << std::endl;
            closeSocket(clientSockets, client.fd);
            return;
        }
        client.totalSent += sent;
    }
    if (client.totalSent >= response.size())
    {
        client.totalSent = 0; 
        client.buffer.clear();
        changePollEvent(client.fd, POLLIN);
    }
    std::cout << "Response sent to " << client.host << ":" << client.port << "\n\n";
    // closeSocket(clientSockets, client.fd); IF Connection keep alive don't close
}

void Server::handleClientSocket(size_t& index)
{
    int fd = pollFds[index].fd;
    Socket *client = findSocket(clientSockets, fd);
    
    if (!client)
    {
        pollFds.erase(pollFds.begin() + index);
        index--;
        return;
    }
    if (pollFds[index].revents & (POLLERR | POLLHUP))
    {
        handleSocketError(fd, index, false);
        return;
    }
    if (pollFds[index].revents & POLLIN)
        readFromClient(*client);    
    client = findSocket(clientSockets, fd);
    if (!client)
        return;
    if (pollFds[index].revents & POLLOUT)
        writeToClient(*client);
}

void Server::run()
{
    initPollFds();
    while (true)
    {
        if (pollFds.empty())
            throw std::runtime_error("Server Error: No sockets to poll");
        if (poll(&pollFds[0], pollFds.size(), -1) == -1)
        {
            closeAllSockets(clientSockets);
            closeAllSockets(listenSockets);
            throw std::runtime_error("Server Error: poll failed");
        }
        for (size_t i = 0; i < pollFds.size(); i++)
        {
            if (pollFds[i].revents == 0)
                continue;
            if (findSocket(listenSockets, pollFds[i].fd) != NULL)
                handleListenSocket(i);
            else
                handleClientSocket(i);
        }
    }
}
