/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/19 16:50:16 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/20 15:10:50 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <sstream>
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

# define CGI_TIMEOUT 5
# define BUFFER_SIZE 4096
# define POLL_TIMEOUT 1000
# define CLIENT_TIMEOUT 300

# define LOG_INFO() std::cout << __FILE__ << ":" << __LINE__ << " " << __func__<< ": ";

class HttpRequest;

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
	HttpRequest *request;
	bool closeAfterResponse;

};

struct CgiConnection
{
	Socket				*client;
	int						stdinFd;
	int						stdoutFd;
	std::time_t		startTime;
};


class Server
{
	public:
		Server(const HttpConfig& config);
		~Server();
		void run();
		void handlegiPollEvent(size_t pollIndex);
		std::vector<Socket> getListenSockets() const;
		CgiConnection* findCgiConnectionByFd(int fd);
		void removeCgiConnection(CgiConnection &conn);
		void handleCgiPollEvent(size_t pollIndex);
			
	private:
		HttpConfig _config;
		std::vector<Socket> _listenSockets;
		std::vector<Socket> _clientSockets;
		std::vector<struct pollfd> _pollFds;
		std::vector<CgiConnection> _cgiConnections;
		
		void closeAllSockets(std::vector<Socket>& sockets);
		void closeSocket(std::vector<Socket>& sockets, int fd);
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
		void removePollFd(int fd);
		bool validateRequestLine(const std::string& line);
		bool validateHeaders(const std::string& headers);
		bool isMalformedRequest(const std::string& buffer);
		void build400AndClose(Socket& client);

};

std::ostream& operator<< (std::ostream &out, const Server& data);

#endif
