/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 16:38:50 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 20:20:30 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <sstream>
#include "utils.hpp"
#include "RequestStatus.hpp"

class HttpRequestParser
{
public:
    HttpRequestParser(const std::string &rawRequest);

    RequestStatus parse();
    const std::string &getMethod() const;
    const std::string &getUri() const;
    const std::string &getHttpVersion() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getBody() const;

private:
    std::istringstream stream;
    std::string method;
    std::string uri;
    std::string httpVersion;
    std::map<std::string, std::string> headers;
    std::string body;

    RequestStatus parseRequestLine();
    RequestStatus parseHeaders();
    RequestStatus parseBody();
};
