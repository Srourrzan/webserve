/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 20:24:25 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 18:49:43 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include <cstdlib>
#include <cerrno>

std::string intToString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

unsigned long strToUL(const std::string& str)
{
    std::string s = trim(str);
    if (s.empty())
        return 0;
    const char* cstr = s.c_str();
    char* endptr = NULL;
    errno = 0;
    unsigned long val = std::strtoul(cstr, &endptr, 10);
    if (endptr == cstr)
        return 0;
    return val;
}

std::string trim(const std::string& line)
{
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    size_t last = line.find_last_not_of(" \t");
    return line.substr(first, last - first + 1);
}

std::string joinPath(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;
    std::string left = a;
    std::string right = b;
    while (!left.empty() && left[left.length() - 1] == '/')
        left = left.substr(0, left.length() - 1);
    while (!right.empty() && right[0] == '/')
        right = right.substr(1);
    return left + "/" + right;
}

bool dirExists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool fileExists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool hasAccess(const std::string& path, int mode)
{
    return access(path.c_str(), mode) == 0;
}

std::string buildPath(const std::string& root, const std::string& locationPath, const std::string& uri)
{
    std::string uriNoPrefix = uri;
    if (!locationPath.empty() && uri.find(locationPath) == 0)
    {
        uriNoPrefix = uri.substr(locationPath.length());
        if (uriNoPrefix.empty())
            uriNoPrefix = "/";
    }
    if (!uriNoPrefix.empty() && uriNoPrefix[0] == '/' && uriNoPrefix != "/")
        uriNoPrefix = uriNoPrefix.substr(1);
    if (root[root.size() - 1] == '/')
        return root + uriNoPrefix;
    return root + "/" + uriNoPrefix;
}

void stripCRLF(std::string& body)
{
    size_t pos = 0;
    while ((pos = body.find("\r\n", pos)) != std::string::npos)
    {
        body.erase(pos, 1);
        pos++;
    }
}

bool isValidFileName(std::string& fileName)
{
    if (fileName.empty() || fileName[fileName.size() - 1] == '/' ||
        fileName.find('.') == std::string::npos)
        return false;
    return true;
}
