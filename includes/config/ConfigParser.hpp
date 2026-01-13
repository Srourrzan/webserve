/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dikhalil <dikhalil@student.42amman.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 00:00:00 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/31 20:33:46 by dikhalil         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "ConfigValidator.hpp"
#include <stdexcept>
#include <sstream>

class ConfigParser
{
    private:
        HttpConfig config;
        ConfigTokenizer tokenizer;
        ConfigValidator validator;
        
        void parseHttp();
        bool parseContext(const std::string& dir, ConfigContext& ctx);
        void parseBodySize(ConfigContext& ctx);
        void parseErrorPage(ConfigContext& ctx);
        void parseReturn(LocationConfig& loc);
        
        template<typename Handler>
        void parseBlock(ConfigContext& ctx, Handler& handler)
        {
            tokenizer.expect("{");
            while (tokenizer.hasMore() && tokenizer.peek() != "}")
            {
                std::string directive = tokenizer.peek();
                
                if (!handler(this, directive) && !parseContext(directive, ctx))
                    throw std::runtime_error("Unknown directive in block: " + directive);
            }
            tokenizer.expect("}");
        }
        
        void parseBool(const std::string& directive, int& target);
        void applyDefaults();
        std::string getValue(const std::string& directive);
        int parsePort(const std::string& port) const;
        long long parseSize(const std::string& val) const;
        ListenConfig parseListen(const std::string& val) const;
        
    public:
        ConfigParser();
        void parse(const std::string& filename);
        const HttpConfig& getConfig() const;
        void parseServer();
        void parseLocation(ServerConfig& server);
        bool parseLocDirective(const std::string& dir, LocationConfig& loc);
        void parseListen(ServerConfig& srv);
        void parseString(const std::string& directive, std::vector<std::string>* target = NULL, std::string *single = NULL);
};

#endif
