/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/20 20:24:25 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/20 23:06:17 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

unsigned long strToUL(const std::string& str)
{
    std::istringstream iss(str);
    unsigned long value = 0;
    iss >> value;
    return value;
}

std::string trim(const std::string& line)
{
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    size_t last = line.find_last_not_of(" \t");
    return line.substr(first, last - first + 1);
}