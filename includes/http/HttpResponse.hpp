///* ************************************************************************** */
///*                                                                            */
///*                                                        :::      ::::::::   */
///*   HttpResponse.hpp                                   :+:      :+:    :+:   */
///*                                                    +:+ +:+         +:+     */
///*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
///*                                                +#+#+#+#+#+   +#+           */
///*   Created: 2025/12/26 18:20:42 by dikhalil          #+#    #+#             */
///*   Updated: 2026/01/01 14:46:32 by dikhalil         ###   ########.fr       */
///*                                                                            */
///* ************************************************************************** */

//#ifndef HTTP_RESPONSE_HPP
//#define HTTP_RESPONSE_HPP

//#include "HttpRequest.hpp"
//#include <map>

//class HttpResponse
//{
//    public:
//		HttpResponse(const HttpRequest &request);
//		const std::string getResponse() const;

//    private:
//		const HttpRequest& request;
//		std::string response;
//		std::string statusLine;
//		std::map<std::string, std::string> header;
//		std::string body;

//		void buildResponse();
//		void buildStatusLine()
//		{
//			std::ostringstream out;

//			out << "HTTP/1.1" << " " << request.getStatus() << " " ;
//			switch (request.getStatus())
//			{
//				case 200:
//					out << "OK";

//			}
//			response = out.str();
//		}

//		void buildHeader()
//		{
//			request.get
//			//content type
//			//content length
//			//connection
//			//search Content-Type
//			//request.getHeaders()["Content-Type"] =
//			std::string value ;

//			std::ostringstream str;

//			str << body.size();

//			header["Content-Type"] = value;
//			header["Connection"] = "keep-alive";
//			header["Content-length"] = str.str();


//		}
//		void buildBody()
//		{
//			if (request.getMethod()  == "POST" || request.getMethod()  == "delete")
//				return ;
//			//redirection 300 301 302
//			std::ifstream file(request.getFinalPath());
//			if (!file)
//			{
//				return "<html>request.getStatus()</html>";
//			}
//			std::ostringstream out;

//			out << file.rdbuf();
//			body = out.str();
//		}

//		const std::string &getContentType() const
//		{

//			if (request.getFinalPath() == ".html")
//				return "text/html";



//		}
//		/*
//			example:

//			HTTP/1.1 200 OK\r\n
//			Content-Type: text/html\r\n
//			Content-Length: 1234\r\n
//			Connection: close\r\n
//			\r\n
//			<!DOCTYPE html>
//			<html>
//			<head>
//				<title>Example Page</title>
//			</head>
//			<body>
//				<h1>Hello, World!</h1>
//				<p>This is an example page.</p>
//			</body>
//			</html>
//		*/

//		/*
//			status line and each line in the header should end with \r\n

//			Connectoin if not found in my request header then the defualt is keep-alive
//			Content-Type set based on the return content

//			to set the body use std::ifstream to open the file for read and
//			std::ostringstream out; out << file.rdbuf() to write all the content from
//			the file buffer to the body; body = out.str();

//			if the file not found(!file) then write any defualt html script
//			based on the error status


//		*/
//};

//#endif
