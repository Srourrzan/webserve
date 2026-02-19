#pragma once
#include <string>
#include "HttpRequest.hpp"

class HttpResponse
{
private:
	std::string fullResponse;
	std::string path;
	std::string header;
	std::string body;
	int codeStatus;
	std::string buildTree(const std::string &path, const std::string &uri);
	std::string contentOfFile;
	bool fileIsComplete;

public:
	HttpResponse();
	~HttpResponse() {}
	bool canReadBody() const;
	void buildResponse(HttpResponse &response, HttpRequest &req);
	std::string getStatusMsg(int code);
	void fileToString(HttpResponse &response, HttpRequest &req, std::string path);
	std::string getContentType(std::string path);
	std::string getHeader() const;
	std::string getBody() const;
	std::string getPath() const;
	int getCodeStatus() const;
	void setCodeStatus(int input);
	void buildCgiResponse(HttpRequest &req);
	std::string getFullResponse() const;
	std::string generateAutoIndex(const std::string &path, const std::string &uri);
};

std::ostream &operator<<(std::ostream &out, const HttpResponse &data);