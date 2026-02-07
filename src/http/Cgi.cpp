#include "Cgi.hpp"
#include "HttpRequest.hpp"
#include "RequestStatus.hpp"
#include <sys/wait.h>   

// HTTP_USER_AGENT	The user agent string of the client's browser,
// which can be used for browser-specific logic.
// SERVER_NAME	The server's hostname or IP address.

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
	cgiEnv["PATH_INFO"] = (q != std::string::npos) ? uri.substr(0, q) : uri;		//
	if (request.getHeaders().count("Content-Length"))
		cgiEnv["CONTENT_LENGTH"] = request.getHeaders().at("Content-Length");
	if (request.getHeaders().count("Content-Type"))
		cgiEnv["CONTENT_TYPE"] = request.getHeaders().at("Content-Type");
	cgiEnv["SERVER_PORT"] = intToString(request.getLocalPort());
	cgiEnv["REMOTE_ADDR"] = request.getLocalIp();//
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
		LOG_INFO();
		std::cout << "script path "
							<< script
							<< std::endl;
		LOG_INFO();
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
	const std::string &body = req.getBody();
	if (!body.empty())
	{
		size_t totalWritten = 0;
		while(totalWritten < body.size())
		{
			ssize_t bytesWritten = write(stdin_fds[1], 
																	body.c_str() + totalWritten, 
																	body.size() - totalWritten);
			if (bytesWritten <= 0)
			{
				if (errno == EINTR)
					continue;
				perror("write to CGI stdin");
				break;
			}
			totalWritten += bytesWritten;
		}
	}
	close(stdin_fds[1]);
	int readFlags = fcntl(stdout_fds[0], F_GETFL, 0);
	fcntl(stdout_fds[0], F_SETFL, readFlags | O_NONBLOCK);
	char buffer[BUFFER_SIZE];
	cgiOutput.clear();
	while (true)
	{
		int bytesRead = read(stdout_fds[0], buffer, BUFFER_SIZE);
		if (bytesRead > 0)
		{
			req.getCgi().cgiOutput.append(buffer, bytesRead);
		}
		else if (bytesRead == 0)
		{
			break;
		}
		else
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// wait(NULL);
				usleep(1000); // should I keep it or remove it?
				continue;
			}
			if (errno == EINTR)
				continue;
			perror("read from CGI");
			break;
		}
	}
	close(stdout_fds[0]);
	int status;
	waitpid(pid, &status, 0);
	LOG_INFO()
	std::cout << "cgi output: " 
						<< req.getCgi().cgiOutput << std::endl;
	req.cgiFlag = true;
	if (WIFEXITED(status))
	{
		int exitCode = WIFEXITED(status);
		if (exitCode != 0)
		{
			LOG_ERR();
			std::cerr << "CGI process for "
								<< req.getFinalPath()
								<< " excited with code: "
								<< exitCode
								<< std::endl;
			req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
			return ;
		}
		else if (WIFSIGNALED(status))
		{
			std::cerr << "CGI process terminated by signal: "
								<< WTERMSIG(status)
								<< std::endl;
			req.setStatus(static_cast<RequestStatus>(REQ_INTERNAL_SERVER_ERROR));
			return ;
		}
		const std::string& cgiOutput = req.getCgi().cgiOutput;
		const std::string separator = "\r\n\r\n";
		size_t sep_pos = cgiOutput.find(separator);
		LOG_INFO();
		std::cout << "sep pos: "
							<< sep_pos
							<< std::endl;
		std::string rawHeaders;
		std::string responseBody;
		if (sep_pos != std::string::npos)
		{
			rawHeaders = cgiOutput.substr(0, sep_pos);
			responseBody = cgiOutput.substr(sep_pos + separator.length());
			// req.getCgi().processCgiHeaders(req, rawHeaders)
		}
	}
}

// void Cgi::parseCgi(HttpRequest& req)
// {
// 		// std::string & body = req.getCgi().cgiOutput;
	
// }

// event loop:
//   ├─ poll()
//   ├─ fd readable?
//   │    └─ read()
//   ├─ append output
//   ├─ read == 0 ?
//   │    └─ CGI done → parse headers/body