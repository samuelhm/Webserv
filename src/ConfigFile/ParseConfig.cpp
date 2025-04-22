/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 14:48:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/14 11:07:43 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseConfig.hpp"
#include "ParseLocation.hpp"
#include "Location.hpp"
#include <stdlib.h>
#include <map>
#include <string>

std::vector<Server*>	parseConfigFile(const str &filepath) {
	str content = Utils::fileToStr(filepath); // si falla lanza una excepcion, creo que es mejor recogerla en el main
	strVec serverStrings = Utils::split(content, '}');
	if (serverStrings.empty())
		throw ConfigFileException("No Server found inside config file");
	std::vector<Server*> result;
	for (strVecIt it = serverStrings.begin(); it != serverStrings.end(); ++it) {
		try {
			(*it) = Utils::trim(*it);
			if ((*it).empty())
				continue;
			Logger::log("Trying to parser a server", INFO);
			Server* server = getServer(*it); //IMPORTANT no se puede push_back de una.
			Logger::log("Server parsed, pushing it.", INFO);
			for (std::vector<Server*>::iterator ite = result.begin(); ite != result.end(); ++ite) {
				if (**ite == *server) {
					delete server;
					throw ConfigFileException("Hostname and port combination must be unique across all server blocks", result);
				}
			}
			result.push_back(server);
		} catch (ConfigFileException &e) {
			throw ConfigFileException(e.what(), result); // ¿Return aqui y limpiamos memoria, o aceptamos el resto de servers validos?
		}
	}
	return result;
}

void	setValidOption(const str &line, OptionType &type)
{
	if (line.find("server_name:") == 0)//== 0 para que la cadena justo empiece en el principio y no contenga caracteres invalidos. viene trimeada
		type = SERVERNAME;
	else if (line.find("listen:") == 0)
		type = LISTEN;
	else if (line.find("is_default:") == 0)
		type = ISDEFAULT;
	else if (line.find("root:") == 0)
		type = ROOT;
	else if (line.find("client_max_body_size:") == 0)
		type = BODYSIZE;
	else if (line.find("error_") == 0 &&
			line.size() >= 10 &&
			std::isdigit(line[6]) &&
			std::isdigit(line[7]) &&
			std::isdigit(line[8]) &&
			line[9] == ':')
	{
		type = ERRORPAGE;
	}
	else if (line.find("location:") == 0)
		type = LOCATION;
	else
		throw UnknownOptionException(line);
}

void	insertOption(const str &value, int type, Server* server)
{
	if (value.empty()) {
		throw EmptyValueException();
	}
  bool tmp;
	switch (type)
	{
		case SERVERNAME:
			server->setServerName(value);
			break;
		case ISDEFAULT:
			tmp = (value == "yes");
			server->setIsdefault(tmp);
			break;
		case ROOT:
			server->setRoot(value);
			break;
		case BODYSIZE:
			server->setBodySize(std::atoi(value.c_str()));
			break;
		case LISTEN:
			server->setListenValue(value);
			break;
		default:
			throw ConfigFileException("FATAL: " + value);
			break;
	}
}

void parseErrorPage(const str &line, Server *server)
{
	size_t code_start = 6;
	str code_str = line.substr(code_start, 3);
	int code = std::atoi(code_str.c_str());
	str path = Utils::trim(line.substr(line.find(":") + 1));
	if (path.empty())
		throw EmptyValueException();
	if (code < 400 || code > 599)
		throw ConfigFileException("Invalid error code: " + code_str);
	try{
		str errorHtml = Utils::fileToStr(path);
		server->setErrorPages(code, errorHtml);
	}
	catch (std::runtime_error &e) {
		Logger::log(e.what(), WARNING);
	}
}

str buildLocationBlock(str &line, std::istringstream &ss, Server *server)
{
	str locationBlock = line + "\n";
	bool foundClosingBracket = false;
	while (std::getline(ss, line)) {
		line = Utils::trim(line);
		if (line.empty()) { continue; }
		locationBlock += line + "\n";
		if (line == "]") {
			foundClosingBracket = true;
			break;
		}
	}
	if (!foundClosingBracket) {
		delete server;
		throw ConfigFileException("Location block not closed with ']'");
	}
	return locationBlock;
}

bool parseLocationBlock(str &line, std::istringstream &ss, Server *server)
{
	str locationBlock = buildLocationBlock(line, ss, server);
	if (locationBlock.empty()) {
		Logger::log("Empty line inside Location block, ignoring...", WARNING);
		return true;
	}
    Location *location = NULL;
    try {
    	location = getLocation(locationBlock);
    } catch (BadSyntaxLocationBlockException const &e) {
    	Logger::log(str("Location deleted and not included becouse line: ") + e.what(), ERROR);
		if (location != NULL)
			delete location;
    return true;
	}
	if (server->locationExist(*location)) {
		Logger::log(str("Location already exist: ") + location->getRoot(), WARNING);
		delete location;
		delete server;
		throw ConfigFileException("You cannot duplicate locations.");
	}
	if (location != NULL)
	{
		if (location->getBodySize() < 1)
			location->setBodySize(Utils::intToStr(server->getBodySize()));
		server->getLocations().push_back(location);
	}
	return false;
}

void checkRepeat(std::vector<OptionType>& options, OptionType type)
{
	if (type == LOCATION || type == ERRORPAGE)
		return ;
	else if (!Utils::contains(options, type))
		options.push_back(type);
	else
		throw RepeatedOptionException();
}

bool handleServerLine(str &line, std::istringstream &ss, Server *server, std::vector<OptionType>& options)
{
	OptionType type;
	try {
		setValidOption(line, type);
		checkRepeat(options, type);
	}
	catch (UnknownOptionException &e) {
		Logger::log(str("Invalid or Unknown Option: ") + e.what(), WARNING);
		return true;
	}
	catch (RepeatedOptionException &e) {
		throw ConfigFileException(e.what());
	}
	if (type != LOCATION)
	{
		try {
			if ( type != ERRORPAGE) {
				str value = Utils::trim(line.substr(line.find(":") + 1));
				insertOption(value, type, server);
				Logger::log(value + "  value saved on server success.", INFO);
			}
			else
				parseErrorPage(line, server);
		} catch (EmptyValueException &e) {
			Logger::log(str("Ignoring line becouse value is empty: ") + line, WARNING);
			return true;
		} catch (ConfigFileException &e) {
			Logger::log(str("Server being deleted, fatal error: ") + e.what(), ERROR);
			delete server;
			throw ConfigFileException(e.what());
		}
	}
	else
		return parseLocationBlock(line, ss, server);
	return false;
}

Server*	getServer(const str &serverString)
{
	std::istringstream ss(serverString);
	str line;
	std::getline(ss, line);
	std::vector<OptionType> Options;
	line = Utils::trim(line);
	if ( line != "{")
		throw ConfigFileException("Server must be between brackets {}");
	Server* server = new Server();
	while (std::getline(ss, line)) {
		line = Utils::trim(line);
		Logger::log(str("Parsing a server line: ") + line, INFO);
		if (line.empty() || line.at(0) == '#')
		{
			Logger::log(str("Ignoring line: ") + line, WARNING);
			continue ;
		}
		if(handleServerLine(line, ss, server, Options))
			continue;
	}
	if (server->getLocations().empty())
		server->getLocations().push_back(new Location("/")); //IMPORTANT ¿ Aceptamos config file sin locations ?
	return (server);
}

ConfigFileException::~ConfigFileException() throw() {}
ConfigFileException::ConfigFileException(const str &msg) : _msg(msg) {}
ConfigFileException::ConfigFileException(const str &msg, std::vector<Server*> &servers) : _msg(msg), _servers(servers) {}
const char* ConfigFileException::what() const throw() { return _msg.c_str(); }
const std::vector<Server*> &ConfigFileException::getServer() const throw() {return _servers; }

UnknownOptionException::UnknownOptionException(const str &msg) : _msg(msg) {}
UnknownOptionException::~UnknownOptionException(void) throw() {}
const char* UnknownOptionException::what() const throw() { return _msg.c_str(); }

const char* EmptyValueException::what() const throw() { return "Empty value in configuration."; }
const char* RepeatedOptionException::what() const throw() { return "Repeated option inside server block."; }
