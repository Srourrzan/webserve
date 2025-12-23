/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rsrour <rsrour@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 00:00:00 by dikhalil          #+#    #+#             */
/*   Updated: 2025/12/23 15:47:16 by rsrour           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Server.hpp"

int main(int argc, char** argv)
{
    if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
        return 1;
    }
    try
    {
        HttpConfig config;
        if (argc == 1)
            config.createDefaultConfig();
        else
        {
            std::string configFile = argv[1];
            ConfigParser parser;

            parser.parse(configFile);
            config = parser.getConfig();
        }
        Server server(config);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
