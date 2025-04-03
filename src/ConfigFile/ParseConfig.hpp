#pragma once

#include <fstream>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "../WebSrv.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "../Utils/Utils.hpp"

std::vector<Server*>	parseConfigFile(const str &filepath);
Server*					getServer(const str &serverString);
Location				getLocation(const str &locationString);
bool					isValidOption(const str &line, int &type);
void					insertOption(str &value, int type, Server* server);

class ConfigFileException : public std::exception {
	private:
		std::string _msg;
	public:
		ConfigFileException(const std::string &msg) : _msg(msg) {}
		virtual const char *what() const throw() {
			return _msg.c_str();
		}
};

class EmptyValueException : public std::exception
{
	public:
		virtual const char* what() const throw();
};
