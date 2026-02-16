/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 17:37:27 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/16 20:22:00 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <string>
#include "utils.hpp"  
#include "HttpRequest.hpp"
#include "RequestStatus.hpp"

class HttpRequestHandler
{
    public:
        HttpRequestHandler(HttpRequest& req);

        RequestStatus handleRequest();
        const std::string& getFinalPath() const;
        void setErrorPagePath();

    private:
        HttpRequest& req;
        std::string finalPath; 
        std::string root;
        std::string cgiBin;
        std::string uploadPath;
        std::string uri;
        std::string path;
        std::string body;
        const LocationConfig *location;
        bool isCgi;

        bool isCgiRequest();
        RequestStatus handleCgi();
        RequestStatus handleGet();
        RequestStatus handlePost();
        RequestStatus handleDelete(const std::string& path);
        RequestStatus deleteDir(const std::string& dirPath);
        RequestStatus checkIndexFiles(const std::string& dirPath);
        RequestStatus writeToFile(const std::string& body);

        template<typename T>
        bool findErrorPage(const T* block, int status, const std::string& root, std::string& outPath)
        {
            if (!block)
                return false;   

            typename std::map<int, std::string>::const_iterator it = block->ctx.errorPages.find(status);
            if (it != block->ctx.errorPages.end())
            {
                std::string pagePath = it->second;
                outPath = joinPath(root, pagePath);
                if (fileExists(outPath) && hasAccess(outPath, R_OK))
                    return true;
            }
            return false;
        }
};

std::ostream & operator<< (std::ostream & out, const HttpRequestHandler & data);