/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 17:15:41 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/20 20:30:30 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"
#include "ConfigTokenizer.hpp"

ConfigTokenizer::ConfigTokenizer() : pos(0) {}

std::string ConfigTokenizer::removeComments(const std::string& line)
{
    size_t commentPos = line.find('#');
    if (commentPos != std::string::npos)
        return line.substr(0, commentPos);
    return line;
}

std::string ConfigTokenizer::addSpacesBetweenSymbol(const std::string& line)
{
    std::ostringstream oss;
    for (size_t i = 0; i < line.length(); i++)
    {
        char c = line[i];
        if (c == '{' || c == '}' || c == ';')
            oss << " " << c << " ";
        else
            oss << c;
    }
    return oss.str();
}

std::string ConfigTokenizer::stripQuotes(const std::string& token)
{
    if (token.length() > 2 && token[0] == '"' && token[token.length() - 1] == '"')
        return token.substr(1, token.length() - 2);
    return token;
}

void ConfigTokenizer::tokenizeFile(const std::string& filename)
{
    std::ifstream configFile(filename.c_str());
    if (!configFile.is_open())
        throw std::runtime_error("Could not open config file " + filename);
    
    std::string line;
    while (std::getline(configFile, line))
    {
        std::string cleanLine = removeComments(line);
        cleanLine = trim(cleanLine);
        if (cleanLine.empty())
            continue;
        cleanLine = addSpacesBetweenSymbol(cleanLine);
        
        std::stringstream ss(cleanLine);
        std::string token;
        while (ss >> token)
            tokens.push_back(token);
    }
    configFile.close();
    pos = 0;
}

std::vector<std::string>& ConfigTokenizer::getTokens()
{
    return tokens;
}

bool ConfigTokenizer::hasMore() const
{
    return pos < tokens.size();
}

std::string ConfigTokenizer::peek() const
{
    if (!hasMore())
        throw std::runtime_error("Unexpected end of file");
    return tokens[pos];
}

std::string ConfigTokenizer::consume()
{
    if (!hasMore())
        throw std::runtime_error("Unexpected end of file");
    return tokens[pos++];
}

std::string ConfigTokenizer::consumeValue()
{
    return (stripQuotes(consume()));
}

void ConfigTokenizer::expect(const std::string& expected)
{
    std::string token = consume();
    if (token != expected)
    {
        if (expected == ";" && (token == "}" || token == "{" || ConfigValidator::isReserved(token)))
        {
            throw std::runtime_error("Missing semicolon ';' before '" + token + "'");
        }
        throw std::runtime_error("Expected '" + expected + "' but got '" + token + "'");
    }
}
