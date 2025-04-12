/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:20 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/12 13:39:13 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Utils/Utils.hpp"
#include "../WebSrv.hpp"
#include "Server.hpp"
#include "Location.hpp"

class Server;

std::vector<Server*>	parseConfigFile(const str &filepath);
Server*					getServer(const str &serverString);
void					setValidOption(const str &line, OptionType &type);
void					insertOption(const str &value, int type, Server* server);
bool 					handleServerLine(str&, std::istringstream&, Server*);
void 					parseErrorPage(const str &line, Server *server);
bool					parseLocationBlock(str &line, std::istringstream &ss, Server *server);

class ConfigFileException : public std::exception {
	private:
		str _msg;
		std::vector<Server*>		_servers;
	public:
		virtual ~ConfigFileException() throw();
		ConfigFileException(const str &msg);
		ConfigFileException(const str &msg, std::vector<Server*> &servers);
		virtual const char*		what() const throw();
		const std::vector<Server*>	&getServer() const throw();
};

class UnknownOptionException : public std::exception {
	private:
		str _msg;
	public:
    virtual ~UnknownOptionException(void) throw();
		UnknownOptionException(const str &msg);
		virtual const char *what() const throw();
};

class EmptyValueException : public std::exception
{
	public:
		virtual const char* what() const throw();
};
