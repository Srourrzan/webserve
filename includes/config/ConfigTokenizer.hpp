/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTokenizer.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 18:58:19 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 20:35:02 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ConfigTokenizer_HPP
#define ConfigTokenizer_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include "utils.hpp"

class ConfigTokenizer
{
    private:
        std::vector<std::string> tokens;
        size_t pos;
        
        std::string removeComments(const std::string& line);
        std::string addSpacesBetweenSymbol(const std::string& line);
        std::string stripQuotes(const std::string& token);
        
        public:
        ConfigTokenizer();
        
        void tokenizeFile(const std::string& filename);
        std::vector<std::string>& getTokens();
        bool hasMore() const;
        std::string peek() const;
        std::string consume();
        std::string consumeValue();
        void expect(const std::string& expected);
};

#endif