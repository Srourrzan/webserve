/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:04 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/20 12:15:56 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const HttpConfig& config)
{
    listenSockets.clear();
    clientSockets.clear();
    fillListenSockets(config);
    initListenSockets();
}

Server::~Server()
{
    closeAllClientSockets();
    closeAllLientSockets();
}

void Server::closeAllLientSockets()
{
   for (size_t i = 0; i < listenSockets.size(); i++)
   {
       if (listenSockets[i].fd != -1)
           close(listenSockets[i].fd);
   }
   listenSockets.clear();
}
void Server::closeAllClientSockets()
{
    for (size_t i = 0; i < clientSockets.size(); i++)
        close(clientSockets[i].fd);
    clientSockets.clear();
}

void Server::closeClientSocket(int clientFd)
{
    for (size_t i = 0; i < clientSockets.size(); i++)
    {
        if (clientSockets[i].fd == clientFd)
        {
            close(clientFd);
            clientSockets.erase(clientSockets.begin() + i);
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
            const ListenConfig& listen = srv.listen[j];
            Socket ls;
            ls.host = listen.host;
            ls.port = listen.port;
            listenSockets.push_back(ls);
        }
    }
}

const std::vector<Socket>& Server::getListenSockets() const
{
    return listenSockets;
}

const std::vector<Socket>& Server::getClientSockets() const
{
    return clientSockets;
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
                successfulSockets.push_back(newSocket);
        }
        freeaddrinfo(res);
    } 
    if (successfulSockets.empty())
        throw std::runtime_error("No valid listening sockets could be created.");
    listenSockets = successfulSockets;
}

void Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl F_GETFL failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl F_SETFL failed");
}

void Server::initPollFds()
{
    pollFds.clear();

    for (size_t i = 0; i < listenSockets.size(); i++)
    {
        struct pollfd pfd;
        pfd.fd = listenSockets[i].fd;
        pfd.events = POLLIN; 
        pfd.revents = 0;
        pollFds.push_back(pfd);
    }
}

void Server::acceptClient(int listenFd)
{
    Socket cl;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientFd == -1)
    {
        std::cerr << "Server Error: accept failed" << std::endl;
        return;
    }
    setNonBlocking(clientFd);
    cl.fd = clientFd;
    cl.port = ntohs(clientAddr.sin_port);
    cl.host = inet_ntoa(clientAddr.sin_addr);
    clientSockets.push_back(cl);
}

void Server::run()
{
    initPollFds();
    while (true)
    {
        if (poll(&pollFds[0], pollFds.size(), 0) == -1)
        {
            std::cerr << "Server Error: poll failed" << std::endl;
            // throw and cleanup
            //for loop and accept clients
        }
    }
}