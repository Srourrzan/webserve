#pragma once
#include<string>
#include "HttpRequest.hpp"

class HttpResponse{

    public:
    std::string fullResponse; //used
    std::string path; //used
    std::string header;
    std::string body;
    int codeStatus; //used
    
    HttpResponse();
   ~HttpResponse() {}
    bool canReadBody() const ;
    void buildResponse(HttpRequest& req);
    std::string getStatusMsg(int code);
    std::string fileToString(std::string path);
    std::string getContentType(std::string path);
    std::string getFullResponse() const;
    std::string generateAutoIndex(const std::string& path, const std::string& uri);
};