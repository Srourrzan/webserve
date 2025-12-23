#include "HttpRequest.hpp"

HttpRequest::HttpRequest(std::string& buffer, HttpConfig& config) :
    _queryString(buffer), _config(config)
{}

HttpRequest::~HttpRequest()
{}

const std::string& HttpRequest::getMethod() const
{
    return (this->_method);
}

const std::string& HttpRequest::getUri() const
{
    return (this->_uri);
}

const std::string& HttpRequest::getPath() const
{
    return (this->_path);
}

const std::string& HttpRequest::getQueryString() const
{
    return (this->_queryString);
}

const std::string& HttpRequest::getHttpVersion() const
{
    return (this->_httpVersion);
}

const std::string& HttpRequest::getBody() const
{
    return (this->_body);
}

const std::vector<std::string, std::string>& HttpRequest::getHeaders() const
{
    return (this->_headers);
}

const std::vector<std::string, std::string>& HttpRequest::getQueryParams() const
{
    return (this->_queryParams);
}

int HttpRequest::parse()
{
    std::cout << "the received HTTP request: " << std::endl;
    std::cout << this->_queryString << std::endl;
    return (0);
}