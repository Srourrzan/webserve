#include <iostream>

#include "Server.hpp"
#include "utils.hpp"

void socketInfo(Socket *socket)
{
	std::cout << "fd: "
						<< socket->fd
						<< " host: "
						<< socket->host
						<< " port: "
						<< socket->port
						<< " listenFd: "
						<< socket->listenFd
						<< " buffer: "
						<< socket->buffer
						<< " lastActivity: "
						<< socket->lastActivity
						<< " totalSent: "
						<< socket->totalSent
						<< std::endl;
}