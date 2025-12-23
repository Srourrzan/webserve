#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <vector>
# include <string>
# include <map>
# include "ConfigStructures.hpp"

class HttpRequest
{
public:
  HttpRequest(std::string& buffer, HttpConfig& config);
  ~HttpRequest();
  int parse();

	const std::string& getMethod() const;
	const std::string& getUri() const;
	const std::string& getPath() const;
	const std::string& getQueryString() const;
	const std::string& getHttpVersion() const;
	const std::string& getBody() const;
	std::string getHeader(const std::string& key) const;
	const std::vector<std::string, std::string>& getHeaders() const;
	const std::vector<std::string, std::string>& getQueryParams() const;
  
  bool isValid() const;
  int getErrorCode() const;

private:
  std::string _uri;
  std::vector<ListenConfig> _host;
  std::string _body;
  std::string _path;
  std::string _method;
  HttpConfig _config;
  std::string _queryString;
  std::string _httpVersion;
  std::vector<std::string, std::string> _headers;
  std::vector<std::string, std::string> _queryParams;
};

#endif
