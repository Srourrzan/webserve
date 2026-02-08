#ifndef CGI_HPP
#define CGI_HPP
#include <map>
#include <string>
#include <sys/types.h>

class HttpRequest;
class Cgi
{
private:
	std::map<std::string, std::string> cgiEnv;
	char **envp;
	std::string cgiOutput;
	std::string cgiHeaders;
	size_t cgiBodySent;
	pid_t pid;
	int stdinFd;
	int stdoutFd;
	bool stdinClosed;
	bool stdoutClosed;

public:
	void buildCgiEnv(HttpRequest &req);
	void setCgi(const Cgi &c);
	char **cgiMaptoChar(std::map<std::string, std::string> &map);
	void prepareCgiEnv(HttpRequest &req);
	void executeCgi(HttpRequest &req);
	void parseCgi(HttpRequest &req);
	void handleCgiBody(HttpRequest &request);
	void handleCgiOutput(HttpRequest &request);
	bool isStdinClosed() const;
    bool isStdoutClosed() const;
};

/*
meta-varibale-name = "AUTH_TYPE" | "CONTENT_LENGTH" |
											"CONTENT_TYPE" | "GATEWAY_INTERFACE" |
											"PATH_INFO" | "PATH_TRANSLATED" |
											"QUERY_STRING" | "REMOTE_ADDR" |
											"REMOTE_HOST" | "REMOTE_IDENT" |
											"REMOTE_USER" | "REQUEST_METHOD" |
											"SCRIPT_NAME" | "SERVER_NAME" |
											"SERVER_PORT" | "SERVER_PROTOCOL" |
											"SERVER_SOFTWARE" | schema |
											protocol-var-name | extension-var-name

protocol-var-name = ( protocol | schema ) "_" var-name
schema = alpha *( alpha | digit | "+" | "-" | ".")
var-name = token
extension-var-name = token
*/

#endif