/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 14:48:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/08 13:57:33 by shurtado         ###   ########.fr       */
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
	std::vector<str> serverStrings = Utils::split(content, '}');
	if (serverStrings.empty())
		throw ConfigFileException("No Server found inside config file");
	std::vector<Server*> result;
	for (std::vector<str>::iterator it = serverStrings.begin(); it != serverStrings.end(); ++it) {
		try {
			(*it) = Utils::trim(*it);
			if ((*it).empty())
				continue;
			Logger::log("Trying to parser a server", INFO);
			Server* server = getServer(*it); //IMPORTANT no se puede push_back de una.
			Logger::log("Server parsed, pushing it.", INFO);
			result.push_back(server);
		} catch (ConfigFileException &e) {
			Logger::log(str("Error parsing server: ") + e.what(), ERROR); // ¿Return aqui y limpiamos memoria, o aceptamos el resto de servers validos?
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
			{
				size_t sep = value.find(":");
				if (sep == std::string::npos)
					throw ConfigFileException("LISTEN must be in format hostname:port " + value); // IMPORTANT check if this information is needeed (to continue or stop)
				str hostname = Utils::trim(value.substr(0, sep));
				str port = Utils::trim(value.substr(sep + 1));
				if (hostname.empty() || port.empty())
					throw ConfigFileException("LISTEN has empty host or port: " + value); // IMPORTANT check if this information is needeed (to continue or stop)
				server->setHostName(hostname);
				server->setPort(port);
				break;
			}
		default:
			throw ConfigFileException("FATAL: " + value);
			break;
	}
}

Server*	getServer(const str &serverString)
{
	std::istringstream ss(serverString);
	str line;
	std::getline(ss, line);
	line = Utils::trim(line);
	if ( line != "{")
		throw ConfigFileException("Server must be between brackets {}");
	Server* server = new Server();
	while (std::getline(ss, line)) {
		line = Utils::trim(line);
		Logger::log(str("Parsing a server line: ") + line, INFO);
		OptionType type;
		if (line.empty() || line.at(0) == '#')
		{
			Logger::log(str("Ignoring line: ") + line, WARNING);
			continue ;
		}
		try { setValidOption(line, type);}
		catch (UnknownOptionException &e){
			Logger::log(str("Invalid or Unknown Option: ") + e.what(), WARNING);
			continue ; //IMPORTANT No estoy cancelando la creacion del server, ignoro la linea.
		}
		if (type != LOCATION) //Todos menos location, que habrá que montar una string con varias lineas
		{
			try {
				if ( type != ERRORPAGE) {
					str value = Utils::trim(line.substr(line.find(":") + 1));
					insertOption(value, type, server);
					Logger::log(value + "  value saved on server success.", INFO);
				}
				else { //Solo ERRORPAGE
					size_t code_start = 6; // pos 6 es el primer numero.
					str code_str = line.substr(code_start, 3); //guardamos los 3 numeros del error.
					int code = std::atoi(code_str.c_str()); // Lo pasamos a integer para el map.
					str path = Utils::trim(line.substr(line.find(":") + 1)); //sacamos el path, desde el : +1.
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
			} catch (EmptyValueException &e) {
				Logger::log(str("Ignoring line becouse value is empty: ") + line, WARNING); //IMPORTANT ¿Excepto localhost:port permitimos empty values y las ignoramos?
				continue ;
			} catch (ConfigFileException &e) {
				Logger::log(str("Server being deleted, fatal error: ") + e.what(), ERROR);
				delete server;
				return NULL;
			}
		}
		else {
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
      		if (locationBlock.empty()) {
				Logger::log("Empty line inside Location block, ignoring...", WARNING);
        		continue;
			}
      		Location *location = NULL;
      		try {
        		location = getLocation(locationBlock, server->getServerName());
      		} catch (BadSyntaxLocationBlockException const &e) {
        		Logger::log(str("Location deleted and not included becouse line: ") + e.what(), ERROR);
			if (location != NULL)
				delete location;
        	continue;
			}
			if (server->locationExist(location)) {
				Logger::log(str("Location already exist: ") + location->getRoot(), WARNING)
				delete location;
				throw ConfigFileException("You cannot duplicate locations.");
			}
			if (location != NULL)
				server->getLocations().push_back(location);
		}
	}
	if (server->getLocations().empty())
		server->getLocations().push_back(new Location(server->getServerName(), "/")); //IMPORTANT ¿ Aceptamos config file sin locations ?
	return (server);
}

const char* EmptyValueException::what() const throw() {	return "You cannot Assign empty value"; }
UnknownOptionException::UnknownOptionException(const std::string &msg) : _msg(msg) {}
const char* UnknownOptionException::what() const throw() { return _msg.c_str();	}
