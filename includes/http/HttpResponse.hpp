#pragma once
#include <string>
#include "HttpRequest.hpp"

class HttpResponse
{
private:
	std::string fullResponse; //used
	std::string path; //used
	std::string header;
	std::string body;
	int codeStatus; //used
	std::string buildTree(const std::string& path, const std::string& uri);

public:
	HttpResponse();
	~HttpResponse() {}
	bool canReadBody() const ;
	void buildResponse(HttpRequest& req);
	std::string getStatusMsg(int code);
	std::string fileToString(std::string path);
	std::string getContentType(std::string path);
	std::string getHeader() const;
	std::string getBody() const;
	std::string getPath() const;
	int getCodeStatus() const;
	std::string getFullResponse() const;
	std::string generateAutoIndex(const std::string& path, const std::string& uri);
};

std::ostream& operator<< (std::ostream &out, const HttpResponse& data);