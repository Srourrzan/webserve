/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/26 18:20:42 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/26 18:21:33 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpRequest.hpp"

class HttpResponse
{
    public:
        HttpResponse();
        ~HttpResponse();

        void setStatus(int status);
        int getStatus() const;

        void setHeader(const std::string& name, const std::string& value);
        std::string getHeader(const std::string& name) const;

        void setBody(const std::string& body);
        std::string getBody() const;

    private:
        int status;
        std::map<std::string, std::string> headers;
        std::string body;
};

#endif 
