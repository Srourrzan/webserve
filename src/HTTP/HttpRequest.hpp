#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <vector>
# include <string>

class HttpRequest
{
public:
  HttpRequest();
  ~HttpRequest();
  int parse(const std::string& rawRequest);

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
  std::string _method;
  std::string _uri;
  std::string _path;
  std::string _queryString;
  std::string _httpVersion;
  std::vector<std::string, std::string> _headers;
  std::vector<std::string, std::string> _queryParams;
  std::string _body;
};

#endif
