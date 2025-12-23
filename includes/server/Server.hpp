/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:16 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/23 15:19:09 by rsrour           ###   ########.fr       */
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

#define BUFFER_SIZE 4096

struct Socket
{
  int fd;
  std::string host;
  int port;
  std::string buffer;
  size_t totalSent;
  std::string _out; //serialized response to send
  int _parsed_requrest_state;
};


class Server
{
    private:
        std::vector<struct pollfd> pollFds;
        std::vector<Socket> listenSockets;
        std::vector<Socket> clientSockets;
        HttpConfig _config;
        
        void closeSocket(std::vector<Socket>& sockets, int fd);
        void closeAllSockets(std::vector<Socket>& sockets);
        bool closeSocketOnError(Socket& ls, const std::string& errorMsg);
        bool setupSocket(Socket& ls, struct addrinfo* addr);
        struct addrinfo* getAddressInfo(const Socket& ls);
        void setNonBlocking(int fd);
        void addToPoll(int fd, short events);
        void initPollFds();
        void changePollEvent(int fd, short events);
        void readFromClient(Socket& client);
        bool requestIsComplete(const std::string& buffer);
        void writeToClient(Socket& client);
        void handleSocketError(int fd, size_t& index, bool isListen);
        void handleListenSocket(size_t& index);
        void handleClientSocket(size_t& index);
        Socket* findSocket(std::vector<Socket>& sockets, int fd);
        void fillListenSockets(const HttpConfig& config);
        void initListenSockets();
        void acceptClient(int listenFd);
        void addClientSocket(int clientFd, struct sockaddr_in& clientAddr);
    public:
        Server(const HttpConfig& config);
        ~Server();
        void run();
};

#endif
