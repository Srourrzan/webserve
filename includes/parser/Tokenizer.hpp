/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 17:15:47 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/23 17:10:31 by rsrour           ###   ########.fr       */
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
        std::string addSpacesBetweenSymbol(const std::string& line);
        std::string stripQuotes(const std::string& token);
        
public:
        Tokenizer();
        
        void tokenizeFile(const std::string& filename);
        std::vector<std::string>& getTokens();
        bool hasMore() const;
        std::string peek() const; //return element without moving
        std::string consume(); //returne the element, and move to the next element
        std::string consumeValue();
        void expect(const std::string& expected);// I pass to it what I expeect after this token, an it cmpares if it's the expected one or not.
};

#endif