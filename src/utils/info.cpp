#include <iostream>

#include "Server.hpp"
#include "utils.hpp"

void socketInfo(Socket *socket)
{
	std::cout << "fd: "
						<< socket->fd
						<< "\nhost: "
						<< socket->host
						<< "\nport: "
						<< socket->port
						<< "\nlistenFd: "
						<< socket->listenFd
						<< "\nbuffer: "
						<< socket->buffer
						<< "\nlastActivity: "
						<< socket->lastActivity
						<< "\ntotalSent: "
						<< socket->totalSent
						<< std::endl;
}