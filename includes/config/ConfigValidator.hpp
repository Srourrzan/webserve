/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 20:33:22 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 20:29:14 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_VALIDATOR_HPP
#define CONFIG_VALIDATOR_HPP

#include "ConfigStructures.hpp"
#include "ConfigTokenizer.hpp"
#include <algorithm>

class ConfigValidator
{
    public:
        static void validate(const HttpConfig& config);
        static bool isValidMethod(const std::string& method);
        static bool isValidStatus(int code);
        static bool isValidRedirectCode(int code);
        static bool isValidIP(const std::string& ip);
        static bool isValidPort(const std::string& port);
        static bool isValidHostname(const std::string& hostname);
        static bool isReserved(const std::string& word);
        static bool isBlock(const std::string& token);
        static bool isDigits(const std::string& str);
        static void checkValue(const ConfigTokenizer& ConfigTokenizer, const std::string& directive);
        static void validateMethod(const std::string& method);
        static bool isValidString(const std::string& value, const std::string& allowedChars);
        static void validateContext(const ConfigContext& ctx);
        
        template<typename T>
        static bool isDuplicate(const std::vector<T>& list, const T& value)
        {
            return std::find(list.begin(), list.end(), value) != list.end();
        }
        
        template<typename T>
        static bool hasCommonElement(const std::vector<T>& v1, const std::vector<T>& v2)
        {
            for (size_t i = 0; i < v1.size(); i++)
            {
                if (isDuplicate(v2, v1[i]))
                    return true;
            }
            return false;
        }
        
        template<typename T, typename CompareFunc>
        static void compareAllPairs(const std::vector<T>& vec, CompareFunc compare)
        {
            for (size_t i = 0; i < vec.size(); i++)
            {
                for (size_t j = i + 1; j < vec.size(); j++)
                    compare(vec[i], vec[j]);
            }
        }
        
    private:
        static void validateServer(const ServerConfig& srv);
        static void validateLocation(const LocationConfig& loc);
        static void checkDuplicates(const HttpConfig& config);
        static void checkDupLocations(const HttpConfig& config);
};

#endif
