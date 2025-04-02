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
Server*					GetServer(const str &serverString);
str						GetOption(const str &optionString);
Location				getLocation(const str &locationString);

class ConfigFileException : public std::exception {
	private:
		std::string _msg;
	public:
		ConfigFileException(const std::string &msg) : _msg(msg) {}
		virtual const char *what() const throw() {
			return _msg.c_str();
		}
};
