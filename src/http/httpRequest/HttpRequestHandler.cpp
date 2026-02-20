/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/30 17:37:19 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/19 22:50:20 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequestHandler.hpp"
#include "HttpRequest.hpp"
#include "Cgi.hpp"
#include "HttpResponse.hpp"
#include <sstream>



HttpRequestHandler::HttpRequestHandler(HttpRequest& r): 
	req(r), 
	finalPath(""), 
	root(""), 
	cgiBin(""), 
	uploadPath(""), 
	uri(""), 
	path(""), 
	body(""), 
	location(NULL), 
	_isCgi(false)
{
	body = req.getBody();
	location = req.getLocation();
	if (location)
	{
		uri = req.getUri();
		size_t queryPos = uri.find('?');
		if (queryPos != std::string::npos)
			uri = uri.substr(0, queryPos);
		root = location->ctx.root;
		path = buildPath(root, location->path, uri);
		cgiBin = location->ctx.cgiBinPath;
		uploadPath = location->uploadPath;
	}
}

HttpRequestHandler::~HttpRequestHandler()
{}

bool HttpRequestHandler::getIsCgi()
{
	return (this->_isCgi);
}

RequestStatus HttpRequestHandler::handleRequest()
{
	if (!dirExists(root) || !hasAccess(root, R_OK | X_OK))
		return REQ_INTERNAL_SERVER_ERROR;
	if (req.getMethod() != "POST" && !dirExists(path) && !fileExists(path))
		return REQ_NOT_FOUND;
	if (isCgiRequest())
	{
		return handleCgi();
	}
	if (req.getMethod() == "GET")
		return handleGet();
	else if (req.getMethod() == "POST")
		return handlePost();
	else if (req.getMethod() == "DELETE")
		return handleDelete(path);
	return REQ_METHOD_NOT_ALLOWED;
}

bool HttpRequestHandler::isCgiRequest()
{
	if (!location->cgiEnabled)
		return false;
	size_t dotPos = path.rfind('.');
	if (dotPos == std::string::npos)
		return false;
	std::string ext = path.substr(dotPos + 1);
	for (size_t i = 0; i < location->cgiExtensions.size(); i++)
	{
		std::string stored = location->cgiExtensions[i];
		if (ext == stored || ("." + ext) == stored)
			return true;
	}
	return false;
}

RequestStatus HttpRequestHandler::handleCgi()
{
	if (!cgiBin.empty() && path.find(cgiBin) != 0)
		return REQ_FORBIDDEN;    
	if (!fileExists(path))
		return REQ_NOT_FOUND;
	if (!hasAccess(path, R_OK))
		return REQ_FORBIDDEN;

	finalPath = path;
	req.setFinalPath(finalPath);
	this->_isCgi = true;
	return REQ_OK;
}



RequestStatus HttpRequestHandler::checkIndexFiles(const std::string& dirPath)
{
	for (size_t i = 0; i < location->ctx.index.size(); i++)
	{
		std::string indexPath = joinPath(root, location->ctx.index[i]);
		if (fileExists(indexPath))
		{
			if (!hasAccess(indexPath, R_OK))
				return REQ_FORBIDDEN;
			finalPath = indexPath;
			return REQ_OK;
		}
	}
	if (!hasAccess(dirPath, R_OK | X_OK))
		return REQ_FORBIDDEN;
	if (location->ctx.autoIndex)
	{
		finalPath = dirPath;
		return REQ_OK;
	}
	return REQ_FORBIDDEN;
}

RequestStatus HttpRequestHandler::handleGet()
{
	if (dirExists(path))
		return checkIndexFiles(path);
	if (!hasAccess(path, R_OK))
		return REQ_FORBIDDEN;
	finalPath = path;
	return REQ_OK;
}

RequestStatus HttpRequestHandler::deleteDir(const std::string& dirPath)
{
	DIR* dir = opendir(dirPath.c_str());
	if (!dir)
		return REQ_INTERNAL_SERVER_ERROR;
	if (!hasAccess(dirPath, R_OK | X_OK))
	{
		closedir(dir);
		return REQ_FORBIDDEN;
	}
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
			continue;
		RequestStatus status = handleDelete(dirPath + "/" + name);
		if (status != REQ_NO_CONTENT)
		{
			closedir(dir);
			return status;
		}
	}
	closedir(dir);
	if (rmdir(dirPath.c_str()) != 0)
		return errno == EACCES ? REQ_FORBIDDEN : REQ_INTERNAL_SERVER_ERROR;
	return REQ_NO_CONTENT;
}

RequestStatus HttpRequestHandler::handleDelete(const std::string& path)
{
    if (dirExists(path))
        return deleteDir(path);
    if (!hasAccess(path, W_OK))
        return REQ_FORBIDDEN;
    if (remove(path.c_str()) != 0)
        return REQ_INTERNAL_SERVER_ERROR;
    return REQ_NO_CONTENT;
}

RequestStatus HttpRequestHandler::writeToFile(const std::string& body)
{
	int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
		return errno == EACCES ? REQ_FORBIDDEN : REQ_INTERNAL_SERVER_ERROR;
	size_t written = write(fd, body.c_str(), body.size());
	close(fd);
	if (written != body.size())
		return REQ_INTERNAL_SERVER_ERROR;
	finalPath = path;
	return REQ_CREATED;
}

RequestStatus HttpRequestHandler::handlePost()
{

	if (!location->uploadEnabled)
		return REQ_METHOD_NOT_ALLOWED;

	std::string uploadDir = joinPath(root, uploadPath);
	if (path.find(uploadDir) != 0)
		return REQ_FORBIDDEN;

	std::string fileName = path.substr(uploadDir.length());
	if (!fileName.empty() && fileName[0] == '/')
		fileName = fileName.substr(1);
	if (fileName.empty())
		return REQ_BAD_REQUEST; 

	if (!dirExists(uploadDir) || !hasAccess(uploadDir, W_OK | X_OK))
		return REQ_FORBIDDEN;

	path = joinPath(uploadDir, fileName);

	if (fileExists(path) && !hasAccess(path, W_OK))
		return REQ_CONFLICT;

	return writeToFile(body);
}

void HttpRequestHandler::setErrorPagePath()
{
	std::string r = (req.getLocation() ? req.getLocation()->ctx.root : 
	(req.getServer() ? req.getServer()->ctx.root : req.getHttpConfig().ctx.root));

	if (findErrorPage(req.getLocation(), req.getStatus(), r, finalPath)) return;
	if (findErrorPage(req.getServer(), req.getStatus(), r, finalPath)) return;
	if (findErrorPage(&req.getHttpConfig(), req.getStatus(), r, finalPath)) return;
}

std::ostream& operator<< (std::ostream &out, const HttpRequestHandler& data)
{
	out << "Http request handler: \n"
			<< "final path "
			<< data.getFinalPath();
	return (out);
}

const std::string& HttpRequestHandler::getFinalPath() const
{
	return finalPath;
}
