/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 20:27:57 by dikhalil          #+#    #+#             */
/*   Updated: 2026/01/17 15:23:15 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

# include <string>
# include <sstream>
# include <sys/stat.h>
# include <unistd.h>
# include "Server.hpp"

struct Socket;

unsigned long   strToUL(const std::string& str);
std::string     trim(const std::string& line);
bool            dirExists(const std::string& path);
bool            fileExists(const std::string& path);
bool            hasAccess(const std::string& path, int mode);
std::string     buildPath(const std::string& root, const std::string& locationPath, const std::string& uri);
std::string     joinPath(const std::string& a, const std::string& b);
void            stripCRLF(std::string& body);
bool            isValidFileName(std::string& fileName);
void            socketInfo(Socket *socket);

#endif