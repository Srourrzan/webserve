#ifndef CGI_HPP
# define CGI_HPP

# include <map>
# include <string>
# include <sys/types.h>

class HttpRequest;

class Cgi
{
public:
	Cgi();
	~Cgi();
	void buildCgiEnv(HttpRequest &req);
	// void setCgi(const Cgi &c);
	char **cgiMaptoChar(std::map<std::string, std::string> &map);
	void prepareCgiEnv(HttpRequest &req);
	void executeCgi(HttpRequest &req);
	void parseCgi(HttpRequest &req);
	void handleCgiBody(HttpRequest &request);
	void handleCgiOutput(HttpRequest &request);
	bool isStdinClosed() const;
	bool isStdoutClosed() const;
	int getStdoutFd() const;
	std::string getCgiOutput() const;
	int getStdinFd() const;
	void setCgiHeaders(std::string input);
	void setCgiBody(std::string input);
	std::string getCgiHeaders() const;
	std::string getCgibody() const;
	void setContentType(std::string input);
	std::string getContentType();
	pid_t getPid() const;
	
private:
	std::map<std::string, std::string> cgiEnv;
	pid_t pid;
	char **envp;
	std::string cgiOutput;
	size_t cgiBodySent;
	int stdinFd;
	int stdoutFd;
	bool stdinClosed;
	bool stdoutClosed;
	std::string cgiHeaders;
	std::string cgiBody;
	std::string contentType;

};

#endif