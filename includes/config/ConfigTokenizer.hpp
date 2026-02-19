/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigTokenizer.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 18:58:19 by dikhalil          #+#    #+#             */
/*   Updated: 2026/02/19 23:30:30 by rsrour           ###   ########.fr       */
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
public:
	ConfigTokenizer();
	~ConfigTokenizer();
	bool hasMore() const;
	std::string consume();
	std::string peek() const;
	std::string consumeValue();
	std::vector<std::string>& getTokens();
	void tokenizeFile(const std::string& filename);
	void expect(const std::string& expected);

private:
	size_t pos;
	std::vector<std::string> tokens;
	std::string removeComments(const std::string& line);
	std::string stripQuotes(const std::string& token);
	std::string addSpacesBetweenSymbol(const std::string& line);

};

#endif