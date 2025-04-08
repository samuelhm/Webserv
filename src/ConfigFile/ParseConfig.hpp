#pragma once

#include <fstream>
#include <exception>
#include <vector>
#include <sstream>
#include "../WebSrv.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "../Utils/Utils.hpp"

std::vector<Server*>	parseConfigFile(const str &filepath);
Server*					getServer(const str &serverString);
void					setValidOption(const str &line, OptionType &type);
void					insertOption(const str &value, int type, Server* server);

class ConfigFileException : public std::exception {
	private:
		std::string _msg;
		std::vector<Server*>		_servers;
	public:
		virtual ~ConfigFileException() throw();
		ConfigFileException(const std::string &msg);
		ConfigFileException(const std::string &msg, std::vector<Server*> &servers);
		virtual const char*		what() const throw();
		const std::vector<Server*>	&getServer() const throw();
};

class UnknownOptionException : public std::exception {
	private:
		std::string _msg;
	public:
    virtual ~UnknownOptionException(void) throw();
		UnknownOptionException(const std::string &msg);
		virtual const char *what() const throw();
};

class EmptyValueException : public std::exception
{
	public:
		virtual const char* what() const throw();
};
