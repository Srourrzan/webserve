#include <iostream>

int main() {
  std::cout << "Content-type: text/html/\n\n";
	std::cout << "<html><body>";
	std::cout << "<h1>Hello, CGI</h1>";
	std::cout << "</body></html>";
	return (0);
}