#include "HttpRequest.hpp"

HttpRequest::HttpRequest() //maybe add init values lateron
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

const std::vector<std::string, std::string>& HttpRequest::