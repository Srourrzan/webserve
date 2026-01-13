/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestValidator.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 17:04:05 by dikhalil          #+#    #+#             */
/*   Updated: 2026/01/08 21:48:13 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestStatus.hpp"
#include "HttpRequest.hpp"
#include "utils.hpp"


class HttpRequestValidator
{
    public:
        HttpRequestValidator(const HttpRequest& req);

        RequestStatus validate();  
        const std::string& getBody() const;
        const ServerConfig* getServer() const;
        const LocationConfig* getLocation() const;
        short getRedirectCode() const;
        const std::string& getRedirectUri() const;

    private:
        const HttpRequest& req;
        const ServerConfig* server;
        const LocationConfig* location;
        short redirectCode;
        std::string redirectUri;
        std::string body;

        RequestStatus validateRequestLine();
        RequestStatus validateHeaders();
        RequestStatus validateBody();

        RequestStatus validateHost();
        RequestStatus checkRedirection();
        RequestStatus checkAllowedMethods();
        RequestStatus validateContentLength();
        RequestStatus validateChunkedEncoding();
        bool unchunkBody(std::string &body);
};

