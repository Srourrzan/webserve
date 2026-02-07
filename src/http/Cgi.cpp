#include "Cgi.hpp"
#include "HttpRequest.hpp"


void Cgi::buildCgiEnv(HttpRequest& request)
{
    cgiEnv.clear();
    cgiEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgiEnv["SERVER_SOFTWARE"] = "webserv/1.0";
    cgiEnv["REQUEST_METHOD"] = request.getMethod();
    cgiEnv["SERVER_PROTOCOL"] = request.getHttpVersion();
    cgiEnv["SCRIPT_NAME"] = request.getUri();
    cgiEnv["SCRIPT_FILENAME"] = request.getFinalPath();

    std::string uri = request.getUri();
    size_t q = uri.find('?');
    cgiEnv["QUERY_STRING"] = (q != std::string::npos) ? uri.substr(q + 1) : "";
    cgiEnv["PATH_INFO"] = (q != std::string::npos) ? uri.substr(0, q) : uri;

    if (request.getHeaders().count("Content-Length"))
        cgiEnv["CONTENT_LENGTH"] = request.getHeaders().at("Content-Length");

    if (request.getHeaders().count("Content-Type"))
        cgiEnv["CONTENT_TYPE"] = request.getHeaders().at("Content-Type");

    // cgiEnv["SERVER_NAME"] = request.getServer() ? request.getServer()->host : "localhost";
    cgiEnv["SERVER_PORT"] = intToString(request.getLocalPort());
    cgiEnv["REMOTE_ADDR"] = request.getLocalIp();

    cgiEnv["REQUEST_URI"] = uri;
    cgiEnv["DOCUMENT_ROOT"] = request.getLocation() ? request.getLocation()->ctx.root : "";
}

void Cgi::prepareCgiEnv(HttpRequest& req)
{
    
   	req.getCgi().buildCgiEnv(req);
	envp = cgiMaptoChar(req.getCgi().cgiEnv);
}

char** Cgi::cgiMaptoChar(std::map <std::string,std::string>& cgiEnv)
{
       size_t size = 0;
    char** envp = new char*[cgiEnv.size() + 1];
    std::map<std::string, std::string>::iterator it;
    for(it = cgiEnv.begin();it != cgiEnv.end();++it)
    {
        std::string entry = it->first + "=" + it->second;
        envp[size] = new char [entry.size() + 1];
        std::strcpy(envp[size], entry.c_str());

        size++;
    }
     envp[size] = NULL;

    return envp;
}

void Cgi::setCgi(const Cgi& c) 
{
    *this = c;
}

void Cgi::executeCgi(HttpRequest& req)
{
	int stdin_fds[2];
	int stdout_fds[2];
	if((pipe(stdin_fds) == -1) || (pipe(stdout_fds) == -1))
	{
		// error code
		return;
	}

	pid_t pid = fork();
	if(pid < 0)
	{
		//close fds, error code , 
		return;
	}

	if(pid == 0)
	{
		close(stdin_fds[1]);
		close(stdout_fds[0]);

		dup2(stdin_fds[0], 0);
		dup2(stdout_fds[1], 1);
		
		close(stdin_fds[0]);
		close(stdout_fds[1]);

		std::string interpreter = "/usr/bin/php-cgi";
        std::string script = req.getFinalPath();
		req.getCgi().prepareCgiEnv(req);

		char* argv[] = {
            const_cast<char*>(interpreter.c_str()),
            const_cast<char*>(script.c_str()),
            NULL
        };
		
		execve(argv[0], argv, req.getCgi().envp);
		perror("execve failed");
        _exit(127);	
	}
		close(stdin_fds[0]);
		close(stdout_fds[1]);
		
		client.cgiPid = pid;
		client.fd = stdout_fds[0];

		int flags = fcntl(stdout_fds[0], F_GETFL, 0);
    	fcntl(stdout_fds[0], F_SETFL, flags | O_NONBLOCK);

		// struct pollfd pfd;
		// pfd.fd     = stdout_fds[0];     
		// pfd.events = POLLIN;        
		// pfd.revents = 0;
		// this->_pollFds.push_back(pfd);
		req.cgiFlag = true;

}
// event loop:
//   ├─ poll()
//   ├─ fd readable?
//   │    └─ read()
//   ├─ append output
//   ├─ read == 0 ?
//   │    └─ CGI done → parse headers/body