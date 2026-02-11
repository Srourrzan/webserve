/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:16 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/07 16:42:18 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>   
#include <errno.h>
#include <iostream>
#include <cstring> 
#include <netdb.h>
#include <sys/types.h>
#include <poll.h>
#include <arpa/inet.h>
#include "HttpRequest.hpp"
#include "ConfigValidator.hpp"

#define BUFFER_SIZE 4096
#define POLL_TIMEOUT 1000
#define CLIENT_TIMEOUT 300

# define LOG_INFO() std::cout << __FILE__ << ":" << __LINE__ << " " << __func__<< ": ";
# define LOG_ERR() std::cerr << __FILE__ << ":" << __LINE__ << " " << __func__<< ": ";

struct Socket
{
    int fd;
    std::string host;
    int port;
    int listenFd;
    std::string buffer;
    std::time_t lastActivity;
    size_t totalSent;
	std::string responseString;

	HttpRequest request; // اجعله يحمل الطلب الحالي
	// bool isCgiReady;
};

class Server
{
public:
	Server(const HttpConfig& config);
	~Server();
	void run();
	std::vector<Socket> getListenSockets() const;
	HttpRequest& req; //added
		
private:
	std::vector<struct pollfd> _pollFds;
	std::vector<Socket> _listenSockets;
	std::vector<Socket> _clientSockets;
	HttpConfig _config;
	
	void closeSocket(std::vector<Socket>& sockets, int fd);
	void closeAllSockets(std::vector<Socket>& sockets);
	bool closeSocketOnError(Socket& ls, const std::string& errorMsg);
	bool setupSocket(Socket& ls, struct addrinfo* addr);
	struct addrinfo* getAddressInfo(const Socket& ls);
	void setNonBlocking(int fd);
	void addToPoll(int fd, short events);
	void initPollFds();
	void checkClientTimeouts();
	void changePollEvent(int fd, short events);
	void readFromClient(Socket& client);
	bool requestIsComplete(const std::string& buffer);
	void writeToClient(Socket& client);
	void handleSocketError(int fd, size_t& index, bool isListen);
	void handleListenSocket(size_t& index);
	void handleClientSocket(size_t& index);
	Socket* findSocket(std::vector<Socket>& sockets, int fd);
	bool isDuplicateSocket(const std::string& host, int port) const;
	void fillListenSockets(const HttpConfig& config);
	void initListenSockets();
	void acceptClient(int listenFd);
	void addClientSocket(int clientFd, int listenFd);
};

std::ostream& operator<< (std::ostream &out, const Server& data);

#endif
