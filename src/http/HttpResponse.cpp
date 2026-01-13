#include "HttpResponse.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>

HttpResponse::HttpResponse() {}
std::string HttpResponse::getStatusMsg(int code)
{
    switch(code) {
        case REQ_OK:                    
            return "OK";
        case REQ_CREATED:               
            return "Created";
        case REQ_NO_CONTENT:            
            return "No Content";
        case REQ_MULTIPLE_CHOICES:      
            return "Multiple Choices";
        case REQ_MOVED_PERMANENTLY:     
            return "Moved Permanently";
        case REQ_FOUND:                 
            return "Found";
        case REQ_BAD_REQUEST:           
            return "Bad Request";
        case REQ_FORBIDDEN:             
            return "Forbidden";
        case REQ_NOT_FOUND:             
            return "Not Found";
        case REQ_METHOD_NOT_ALLOWED:    
            return "Method Not Allowed";
        case REQ_CONFLICT:              
            return "Conflict";
        case REQ_PAYLOAD_TOO_LARGE:     
            return "Payload Too Large";
        case REQ_URI_TOO_LONG:          
            return "URI Too Long";
        case REQ_INTERNAL_SERVER_ERROR: 
            return "Internal Server Error";
        case REQ_NOT_IMPLEMENTED:       
            return "Not Implemented";
        case REQ_VERSION_NOT_SUPPORTED: 
            return "HTTP Version Not Supported";
        default:                        
            return "Unknown Status";
    }
}

std::string HttpResponse::fileToString(std::string path)
{
    std::stringstream ss;
    std::ifstream file(path.c_str(), std::ios::binary);
    if(!file.is_open()){
        std::cerr << "Error : Can not open the file" <<std::endl; //ask
        return ""; 
    }
    ss << file.rdbuf();
    file.close();
    return (ss.str());
}

std::string toLowerStr(std::string str) {
    for (size_t i = 0; i < str.length(); i++) {
        str[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[i])));
    }
    return str;
}

std::string HttpResponse::getContentType(std::string path)
{
    size_t position;
    std::string type;
    position = path.rfind('.');

    if (position == std::string::npos)
        return "text/plain";

    type = path.substr((position + 1), path.size());
    type = toLowerStr(type);

    if (type == "html" || type == "htm")
        return "text/html";
    else if (type == "css")
        return "text/css";
    else if (type == "js")
        return "application/javascript";
    else if (type == "jpg" || type == "jpeg")
        return "image/jpeg";
    else if (type == "png")
        return "image/png";
    else if (type == "gif")
        return "image/gif";
    else if (type == "json")
        return "application/json";

    return "text/plain"; //check
}

std::string HttpResponse::getFullResponse() const {
    return this->fullResponse;
}

std::string intToString( int value) {
    std::stringstream ss; 
    ss << value;          
    return ss.str();     
}

std::string HttpResponse::generateAutoIndex(const std::string& path, const std::string& uri)
{
    std::string fullResponse;
    fullResponse = "<html><head><title>Index of " + uri + "</title></head>";
    fullResponse += "<body style='font-family: sans-serif; padding: 20px;'>";
    fullResponse += "<h1>Index of " + uri + "</h1><hr><ul style='list-style: none;'>";
    
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) 
    {
        return "";
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        std::string link = uri;
        if (link[link.size() - 1] != '/') link += "/";
        link += name;
        fullResponse += "<li><a href=\"" + link + "\">" + name + (entry->d_type == DT_DIR ? "/" : "") + "</a></li>";
    }

    closedir(dir);
    fullResponse += "</ul><hr><p style='font-size: small;'>Webserv/1.0</p></body></html>";
    
    return fullResponse;
}    

void HttpResponse::buildResponse(HttpRequest& req)
{
    this->codeStatus = req.getStatus();
    this->path = req.getFinalPath();
    std::string ConnectionValue;
    const LocationConfig* loc = req.getLocation();

    if(req.getRedirectCode() !=0 )
    {
        this->fullResponse = "HTTP/1.1 " + intToString(this->codeStatus) + " " + getStatusMsg(this->codeStatus) + "\r\n";
        this->fullResponse += "Location: " + req.getRedirectUri()+ "\r\n"; 
        this->fullResponse += "Content-Length: 0\r\n";
        this->fullResponse += "Connection: close\r\n\r\n";
        return;
    }

    else if (this->codeStatus == REQ_OK && dirExists(this->path))
    {
        if (loc != NULL && loc->ctx.autoIndex == 1)
        {
            this->body = generateAutoIndex(this->path, req.getUri());
        }
    }
    else
    {
        this->body = fileToString(path);
        if (this->body.empty() && this->codeStatus >= 400) {
            this->body = "<html><body><h1>" + intToString(this->codeStatus) + " " + getStatusMsg(this->codeStatus) + "</h1></body></html>";
        }
    }


    this->fullResponse = "HTTP/1.1 " + intToString(this->codeStatus) + " " + getStatusMsg(codeStatus) + "\r\n";
    if (this->codeStatus >= 400 || dirExists(this->path))
        this->fullResponse += "Content-Type: text/html\r\n";
    else
        this->fullResponse += "Content-Type: " + getContentType(this->path) + "\r\n";
    
    // this->fullResponse += "Content-Type: " + getContentType(path) + "\r\n";
    this->fullResponse += "Content-Length: " + intToString(this->body.size()) + "\r\n"; 
    this->fullResponse += "Server: Webserv/1.0\r\n"; //ask
    const std::map<std::string,std::string>&h = req.getHeaders();
    std::map<std::string,std::string>::const_iterator it = h.find("Connection");
    
    if(it != h.end())
    {
        ConnectionValue = it->second;
    }
    else
        ConnectionValue = "keep-alive";
    this->fullResponse += "Connection: " + ConnectionValue + " \r\n";   
    this->fullResponse += "\r\n";
    this->fullResponse += this->body;

}