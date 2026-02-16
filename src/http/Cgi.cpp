#include "Cgi.hpp"
#include "HttpRequest.hpp"
#include "RequestStatus.hpp"
#include <sys/wait.h>
#include <string>
// HTTP_USER_AGENT	The user agent string of the client's browser,
// which can be used for browser-specific logic.
// SERVER_NAME	The server's hostname or IP address.

bool Cgi::isStdinClosed() const { return stdinClosed; }

bool Cgi::isStdoutClosed() const { return stdoutClosed; }

void Cgi::setCgiHeaders(std::string input)
{
	this->cgiHeaders = input;
}

void Cgi::setContentType(std::string input)
{
	this->contentType = input;
}

int Cgi::getStdoutFd() const
{
	return this->stdoutFd;
}

int Cgi::getStdinFd() const
{
	return this->stdinFd;
}

std::string Cgi::getContentType()
{
	return this->contentType;
}

std::string Cgi::getCgiOutput() const
{
	return this->cgiOutput;
}

std::string Cgi::getCgibody() const
{
	return this->cgiBody;
}

void Cgi::setCgiBody(std::string input)
{
	this->cgiBody = input;
}

std::string Cgi::getCgiHeaders() const
{
	return this->cgiHeaders;
}

void Cgi::handleCgiBody(HttpRequest &request)
{
	const std::string &body = request.getBody();
	Cgi &cgi = request.getCgi();
	int bytes = write(cgi.stdinFd, body.c_str() + cgi.cgiBodySent, body.size() - cgi.cgiBodySent);
	if (bytes == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;

		std::cerr << "CGI Write Error: Broken Pipe" << std::endl;
		close(cgi.stdinFd);
		cgi.stdinFd = -1;
		request.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}
	else if (bytes > 0)
	{
		cgi.cgiBodySent += bytes;
	}

	else if (cgi.cgiBodySent >= body.size())
	{
		close(cgi.stdinFd);
		cgi.stdinFd = -1;
		cgi.stdinClosed = true;
	}
	std::cout << cgi.cgiBodySent;
}

void Cgi::handleCgiOutput(HttpRequest &request)
{
	Cgi &cgi = request.getCgi();

	char buf[BUFFER_SIZE];
	int bytes = read(cgi.stdoutFd, buf, BUFFER_SIZE);
	if (bytes == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;

		std::cerr << "CGI Read Error: Failed to read from script" << std::endl;
		close(cgi.stdoutFd);
		cgi.stdoutFd = -1;
		request.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}
	else if (bytes == 0)
	{
		close(cgi.stdoutFd);
		cgi.stdoutFd = -1;
		cgi.stdoutClosed = true;

		if (cgi.cgiOutput.empty())
		{
			std::cerr << "CGI Error: Empty response" << std::endl;
			request.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		}
	}
	else if (bytes > 0)
	{
		cgi.cgiOutput.append(buf, bytes);
	}
}
void Cgi::buildCgiEnv(HttpRequest &request)
{
	cgiEnv.clear();
	cgiEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
	cgiEnv["SERVER_SOFTWARE"] = "webserv/1.0";
	cgiEnv["REQUEST_METHOD"] = request.getMethod(); //
	cgiEnv["SERVER_PROTOCOL"] = request.getHttpVersion();
	cgiEnv["SCRIPT_NAME"] = request.getUri(); //
	cgiEnv["SCRIPT_FILENAME"] = request.getFinalPath();
	std::string uri = request.getUri();
	size_t q = uri.find('?');
	cgiEnv["QUERY_STRING"] = (q != std::string::npos) ? uri.substr(q + 1) : ""; //
	cgiEnv["PATH_INFO"] = (q != std::string::npos) ? uri.substr(0, q) : uri;	//
	if (request.getHeaders().count("Content-Length"))
		cgiEnv["CONTENT_LENGTH"] = request.getHeaders().at("Content-Length");
	if (request.getHeaders().count("Content-Type"))
		cgiEnv["CONTENT_TYPE"] = request.getHeaders().at("Content-Type");
	cgiEnv["SERVER_PORT"] = intToString(request.getLocalPort());
	cgiEnv["REMOTE_ADDR"] = request.getLocalIp(); //
	cgiEnv["REQUEST_URI"] = uri;
	cgiEnv["DOCUMENT_ROOT"] = request.getLocation() ? request.getLocation()->ctx.root : "";
}

void Cgi::prepareCgiEnv(HttpRequest &req)
{
	req.getCgi().buildCgiEnv(req);
	envp = cgiMaptoChar(req.getCgi().cgiEnv);
}

char **Cgi::cgiMaptoChar(std::map<std::string, std::string> &cgiEnv)
{
	size_t size = 0;
	char **envp = new char *[cgiEnv.size() + 1];
	std::map<std::string, std::string>::iterator it;
	for (it = cgiEnv.begin(); it != cgiEnv.end(); ++it)
	{
		std::string entry = it->first + "=" + it->second;
		envp[size] = new char[entry.size() + 1];
		std::strcpy(envp[size], entry.c_str());
		size++;
	}
	envp[size] = NULL;
	return envp;
}

void Cgi::setCgi(const Cgi &c)
{
	*this = c;
}

void Cgi::executeCgi(HttpRequest &req)
{
	int stdin_fds[2];
	int stdout_fds[2];
	if ((pipe(stdin_fds) == -1))
	{
		req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}
	if ((pipe(stdout_fds) == -1))
	{
		close(stdin_fds[0]);
		close(stdin_fds[1]);
		req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}
	req.getCgi().prepareCgiEnv(req);
	pid_t pid = fork();
	if (pid < 0)
	{
		close(stdin_fds[0]);
		close(stdin_fds[1]);
		close(stdout_fds[0]);
		close(stdout_fds[1]);
		req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}
	if (pid == 0)
	{
		close(stdin_fds[1]);
		close(stdout_fds[0]);
		if (dup2(stdin_fds[0], STDIN_FILENO) == -1 ||
			dup2(stdout_fds[1], STDOUT_FILENO) == -1)
		{
			perror("dup2 failed");
			_exit(1);
		}
		close(stdin_fds[0]);
		close(stdout_fds[1]);
		std::string interpreter = "/usr/bin/python3";
		std::string script = req.getFinalPath();
		char *argv[] = {
			const_cast<char *>(interpreter.c_str()),
			const_cast<char *>(script.c_str()),
			NULL};
		execve(argv[0], argv, req.getCgi().envp);
		perror("execve failed");
		_exit(127);
	}

	close(stdin_fds[0]);
	close(stdout_fds[1]);
	
	// Close stdin immediately since we're not sending data to the CGI script
	close(stdin_fds[1]);
	
	fcntl(stdout_fds[0], F_SETFL, O_NONBLOCK);
	req.getCgi().pid = pid;
	req.getCgi().stdinFd = -1;
	req.getCgi().stdoutFd = stdout_fds[0];
	req.getCgi().stdinClosed = true;
	req.getCgi().stdoutClosed = false;

	const int CGI_TIMEOUT = 5;
	time_t start = time(NULL);
	int status = 0;
	while (true)
	{
		pid_t ret = waitpid(pid, &status, WNOHANG);
		if (ret == pid)
			break;

		if (time(NULL) - start > CGI_TIMEOUT)
		{
			kill(pid, SIGKILL);
			req.setStatus(static_cast<RequestStatus>(REQ_GATEWAY_TIMEOUT));
			return;
		}

		usleep(10000);
	}

	if (WIFEXITED(status))
	{
		int exitCode = WEXITSTATUS(status);
		if (exitCode != 0)
		{
			req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
			return;
		}
	}
	else
	{
		req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
		return;
	}

	// Read all remaining CGI output after child exits
	if (req.getCgi().stdoutFd != -1)
	{
		int fd = req.getCgi().stdoutFd;
		char buf[BUFFER_SIZE];
		ssize_t n;
		
		// Remove O_NONBLOCK for final blocking read to ensure we get all data
		fcntl(fd, F_SETFL, 0);
		
		while (true)
		{
			n = read(fd, buf, BUFFER_SIZE);
			if (n > 0)
			{
				req.getCgi().cgiOutput.append(buf, n);
			}
			else if (n == 0)
			{
				// EOF reached
				break;
			}
			else
			{
				// Error
				if (errno != EAGAIN && errno != EWOULDBLOCK)
				{
					std::cerr << "CGI Read Error: " << strerror(errno) << std::endl;
				}
				break;
			}
		}
		
		close(fd);
		req.getCgi().stdoutFd = -1;
		req.getCgi().stdoutClosed = true;
		
		std::cout << "[DEBUG CGI] Script output size: " << req.getCgi().cgiOutput.size() << " bytes" << std::endl;
		if (req.getCgi().cgiOutput.size() > 0)
		{
			std::cout << "[DEBUG CGI] First 200 chars: " << req.getCgi().cgiOutput.substr(0, 200) << std::endl;
		}
	}
}