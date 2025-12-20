/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 17:15:47 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/18 01:22:11 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include "webserv.hpp"

class Tokenizer
{
    private:
        std::vector<std::string> tokens;
        size_t pos;
        
        std::string removeComments(const std::string& line);
        std::string trim(const std::string& line);
        std::string addSpacesBetweenSymbol(const std::string& line);
        std::string stripQuotes(const std::string& token);
        
        public:
        Tokenizer();
        
        void tokenizeFile(const std::string& filename);
        std::vector<std::string>& getTokens();
        bool hasMore() const;
        std::string peek() const;
        std::string consume();
        std::string consumeValue();
        void expect(const std::string& expected);
};

#endif