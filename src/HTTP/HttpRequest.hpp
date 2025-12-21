#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <map>
# include <string>

class HttpRequest
{
public:
  HttpRequest();
  ~HttpRequest();
  int parse(const std::string &rawResponse);
  const std::string& getMethod() const;
  const std::string& getUri() const;
  const std::string& getPath() const;
  const std::string& getQueryString() const;
  const std::string& getHttpVersion() const;
  const std::string& getBody() const;
  std::string getHeader(const std::string& key) const;
  const std::map<std::string, std::string>& getHeaders() const;
  const std::map<std::string, std::string>& getQueryParams() const;
  

private:
  std::string _method;
  std::string _uri;
  std::string _path;
  std::string _queryString;
  std::string _httpVersion;
  std::map<std::string, std::string> _headers;
  std::map<std::string, std::string> _queryParams;
  std::string _body;
};

#endif
