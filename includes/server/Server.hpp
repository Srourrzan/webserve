/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:16 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/20 03:08:36 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>    
#include <netdb.h>
#include <sys/types.h>
#include "ConfigStructures.hpp"
#include <poll.h>
#include <arpa/inet.h>

struct Socket
{
    int fd;
    std::string host;
    int port;
};

class Server
{
    private:
        std::vector<struct pollfd> pollFds;
        std::vector<Socket> listenSockets;
        std::vector<Socket> clientSockets;
        
        bool closeSocketOnError(Socket& ls, const std::string& errorMsg);
        bool setupSocket(Socket& ls, struct addrinfo* addr);
        struct addrinfo* getAddressInfo(const Socket& ls);
        void closeAllLientSockets();
        void closeClientSocket(int clientFd);
        void closeAllClientSockets();
        void setNonBlocking(int fd);
        void initPollFds();
        void fillListenSockets(const HttpConfig& config);
        void initListenSockets();
        void acceptClient(int listenFd);
    public:
        Server(const HttpConfig& config);
        ~Server();
        void run();
        const std::vector<Socket>& getListenSockets() const;
        const std::vector<Socket>& getClientSockets() const;
};

#endif
