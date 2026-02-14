#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include "RequestStatus.hpp"

HttpResponse::HttpResponse()
{
    fullResponse = ""; // used
    path = "";         // used
    header = "";
    body = "";
    contentOfFile = "";
    fileIsComplete = false;
}
void HttpResponse::setCodeStatus(int input)
{
    this->codeStatus = input;
}

std::string HttpResponse::getStatusMsg(int code)
{
    switch (code)
    {
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

// void HttpResponse::fileToString(HttpResponse &response, HttpRequest &req, std::string path)
// {
// 	std::ifstream file(path.c_str(), std::ios::binary);
// 	if (!file.is_open())
// 	{
// 		LOG_ERR();
// 		std::cerr << "Error : Can not open the file" << std::endl; // ask
// 		req.setStatus(REQ_NOT_FOUND);
// 		return;
// 	}
// 	char buf[BUFFER_SIZE];
// 	file.read(buf, BUFFER_SIZE);
// 	std::streamsize bytesRead = file.gcount();
// 	if (file.peek() == std::char_traits<char>::eof())
// 	{
// 		response.fileIsComplete = true;
// 		response.contentOfFile = "";
// 		file.close();
// 		LOG_INFO();
// 		std::cout << "file is complete and content of file is empty string"
// 							<< std::endl;
// 		return;
// 	}
// 	if (bytesRead > 0)
// 	{
// 		response.contentOfFile.append(buf, bytesRead);
// 	}
// 	if (file.eof())
// 	{
// 		response.fileIsComplete = true;
// 		file.close();
// 	}
// }

// RAZAN
void HttpResponse::fileToString(HttpResponse &response, HttpRequest &req, std::string path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERR();
        std::cerr << "Error: Cannot open file " << path << std::endl;
        req.setStatus(REQ_NOT_FOUND);
        return;
    }
    
    response.contentOfFile.clear();
    response.fileIsComplete = false;
    
    char buf[BUFFER_SIZE];
    while (true)
    {
        file.read(buf, BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();
        
        if (bytesRead <= 0)
            break;
            
        response.contentOfFile.append(buf, bytesRead);
    }
    
    if (file.bad())
    {
        std::cerr << "Error reading file " << path << std::endl;
        req.setStatus(REQ_INTERNAL_SERVER_ERROR);
        return;
    }
    
    response.fileIsComplete = true;
    file.close();
    
    LOG_INFO();
    std::cout << "Successfully read " << response.contentOfFile.size() << " bytes from " << path << std::endl;
}

std::string toLowerStr(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
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

    return "text/plain"; // check
}

std::string HttpResponse::getFullResponse() const
{
    return this->fullResponse;
}

// std::string intToString( int value) {
//     std::stringstream ss;
//     ss << value;
//     return ss.str();
// }

std::string HttpResponse::buildTree(const std::string &path, const std::string &uri)
{
    std::string html = "<ul>";
    DIR *dir = opendir(path.c_str());
    if (!dir)
        return "";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string pathSep = (path[path.length() - 1] == '/') ? "" : "/";
        std::string uriSep = (uri[uri.length() - 1] == '/') ? "" : "/";

        std::string fullPath = path + pathSep + name;
        std::string fullUri = uri + uriSep + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {

                html += "<li><b style='color: blue;'>[DIR] " + name + "</b>";
                html += buildTree(fullPath, fullUri);
                html += "</li>";
            }
            else
            {
                html += "<li><a href=\"" + fullUri + "\">" + name + "</a></li>";
            }
        }
    }
    closedir(dir);
    html += "</ul>";
    return html;
}

std::string HttpResponse::generateAutoIndex(const std::string &path, const std::string &uri)
{
    std::string content = "<html><head><title>Index of " + uri + "</title></head><body>";

    content += "<h1> Index of " + uri + "</h1><hr>";
    content += buildTree(path, uri);
    content += "<hr></body></html>";
    return content;
}

void HttpResponse::buildCgiResponse(HttpRequest &req)
{
    Cgi cgi;
    cgi = req.getCgi();
    std::string output = cgi.getCgiOutput();

    size_t delimiter = output.find("\r\n\r\n");
    if (delimiter == std::string::npos)
    {
        delimiter = output.find("\n\n");
    }
    size_t statusPos = cgi.getCgiHeaders().find("Status: ");
    if (delimiter != std::string::npos)
    {
        cgi.setCgiHeaders(output.substr(0, delimiter));
        cgi.setCgiBody(output.substr(delimiter + 4));
        if (statusPos != std::string::npos)
        {
            std::string statusVal = cgi.getCgiHeaders().substr(statusPos + 8, 3);
            this->setCodeStatus(std::atoi(statusVal.c_str()));
        }
        else
        {
            this->setCodeStatus(200);
        }
        cgi.setContentType ("text/html");
        size_t typePos = cgi.getCgiHeaders().find("Content-Type:");
        if (typePos != std::string::npos) {
            size_t endLine = cgi.getCgiHeaders().find("\r\n", typePos);
            cgi.setContentType(cgi.getCgiHeaders().substr(typePos + 13, endLine - (typePos + 13)));
        }

        this->fullResponse = "HTTP/1.1 200 OK\r\n"; 
        
        this->fullResponse += "Content-Type: " + cgi.getContentType() + "\r\n";
        this->fullResponse += "Content-Length: " + intToString(cgi.getCgibody().size()) + "\r\n";
        this->fullResponse += "Server: Webserv/1.0\r\n";

    }
   
    
}

void HttpResponse::buildResponse(HttpResponse &response, HttpRequest &req)
{
	this->codeStatus = req.getStatus();
	this->path = req.getFinalPath();
	std::string physicalPath = req.getFinalPath();
	std::string ConnectionValue;
	LOG_INFO();
	std::cout << " physical path: "
						<< physicalPath
						<< std::endl;
	const LocationConfig *loc = req.getLocation();
	LOG_INFO();
	std::cout << " codeStatus: "
						<< this->codeStatus
						<< std::endl;
	LOG_INFO();
	std::cout << " redirect code: "
						<< req.getRedirectCode()
						<< std::endl;
	if (req.getRedirectCode() != 0)
	{
		LOG_INFO();

		this->fullResponse = "HTTP/1.1 " + intToString(this->codeStatus) + " " + getStatusMsg(this->codeStatus) + "\r\n";
		this->fullResponse += "Location: " + req.getRedirectUri() + "\r\n";
		this->fullResponse += "Content-Length: 0\r\n";
		this->fullResponse += "Connection: close\r\n\r\n";
		return;
	}
	else if (this->codeStatus == REQ_OK && dirExists(physicalPath))
	{
		LOG_INFO();
		std::cout << std::endl;
		if (loc != NULL && loc->ctx.autoIndex == 1)
		{
		LOG_INFO();
		std::cout << std::endl;
			this->body = generateAutoIndex(this->path, req.getUri());
		}
	}
	else
	{
		LOG_INFO();
		std::cout << std::endl;
		fileToString(response, req, this->path);
		if (response.fileIsComplete)
		{
			LOG_INFO();
			std::cout << std::endl;
			this->body = contentOfFile; //try response.contentoffile
			LOG_INFO();
			std::cout << "content of file: "
								<< contentOfFile
								<< std::endl;
			LOG_INFO();
			std::cout << "response body: "
								<< this->body
								<< std::endl;
		}
		else if (this->body.empty() && this->codeStatus >= 400)
		{
			LOG_INFO();
			std::cout << std::endl;
			this->body = "<html><body><h1>" + intToString(this->codeStatus) + " " + getStatusMsg(this->codeStatus) + "</h1></body></html>";
		}
		else
			return;
	}
	LOG_INFO();
	this->fullResponse = "HTTP/1.1 " + intToString(this->codeStatus) + " " + getStatusMsg(codeStatus) + "\r\n";
	LOG_INFO();
	if (this->codeStatus >= 400 || dirExists(this->path))
	{
		LOG_INFO();
		this->fullResponse += "Content-Type: text/html\r\n";
	}
	else
	{
		LOG_INFO();
		this->fullResponse += "Content-Type: " + getContentType(this->path) + "\r\n";
	}
	LOG_INFO();

	// this->fullResponse += "Content-Type: " + getContentType(path) + "\r\n";
	this->fullResponse += "Content-Length: " + intToString(this->body.size()) + "\r\n";
	this->fullResponse += "Server: Webserv/1.0\r\n"; // ask
	const std::map<std::string, std::string> &h = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it = h.find("Connection");

	if (it != h.end())
	{
		ConnectionValue = it->second;
	}
	else
		ConnectionValue = "keep-alive";
	this->fullResponse += "Connection: " + ConnectionValue + " \r\n";
	this->fullResponse += "\r\n";
	this->fullResponse += this->body;
	LOG_INFO();
	std::cout << fullResponse << std::endl;
}

std::string HttpResponse::getPath() const
{
    return (this->path);
}

std::string HttpResponse::getHeader() const
{
    return (this->header);
}

std::string HttpResponse::getBody() const
{
    return (this->body);
}

int HttpResponse::getCodeStatus() const
{
    return (this->codeStatus);
}

std::ostream &operator<<(std::ostream &out, const HttpResponse &data)
{
    out << "Http Respnse info:\n"
        << "full reponse:\n"
        << data.getFullResponse()
        << "\npath:\n"
        << data.getPath()
        << "\nheader\n"
        << data.getHeader()
        << "\nbody\n"
        << data.getBody()
        << "\ncodeStatus\n"
        << data.getCodeStatus();
    return (out);
}