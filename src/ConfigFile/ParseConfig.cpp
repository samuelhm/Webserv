/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseConfig.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 14:48:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/06 14:18:57 by shurtado         ###   ########.fr       */
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
      //std::cout << *it << std::endl;
			(*it) = Utils::trim(*it);
			if ((*it).empty())
				continue;
			result.push_back(getServer(*it));
		} catch (ConfigFileException &e) {
			std::cout << "Error parsing server: " << e.what() << std::endl; // ¿Return aqui y limpiamos memoria, o aceptamos el resto de servers validos?
		}
	}
	return result;
}

bool	isValidOption(const str &line, OptionType &type)
{
	bool result = true;
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
		result = false;
	return result;
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
					throw ConfigFileException("LISTEN must be in format hostname:port" + value); // IMPORTANT check if this information is needeed (to continue or stop)
				str hostname = Utils::trim(value.substr(0, sep));
				str port = Utils::trim(value.substr(sep + 1));
				if (hostname.empty() || port.empty())
					throw ConfigFileException("LISTEN has empty host or port: " + value); // IMPORTANT check if this information is needeed (to continue or stop)
				server->setHostName(hostname);
				server->setPort(port);
				break;
			}
		default:
			throw std::exception();
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
		OptionType type;
		if (line.empty())
			continue ;
		if (!isValidOption(line, type)) {
			throw ConfigFileException("Wrong option: " + line); // IMPORTANT check if it should continue or stop and give an error
		}
		if (type != LOCATION) //Todos menos location, que habrá que montar una string con varias lineas
		{
			try {
				if ( type != ERRORPAGE) {
					str value = Utils::trim(line.substr(line.find(":") + 1));
					insertOption(value, type, server);
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
					server->setErrorPages(code, path);
				}
			} catch (EmptyValueException &e) {
				std::cout << e.what() << "ignoring option." << std::endl;
				continue ;
			} catch (...) { throw ConfigFileException("Unknown Error (insertOption())"); }
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
			if (!foundClosingBracket)
				throw ConfigFileException("Location block not closed with ']'");
      if (locationBlock.empty())
        continue;
      Location *location;
      try {
        location = getLocation(locationBlock, server->getServerName());
      } catch (BadSyntaxLocationBlockException const &e) {
        std::cout << e.what() << std::endl;
        continue;
      }
			server->getLocations().push_back(location);
		}
    //if (server->getLocations().empty())
    //  std::cout << "Handle error, server without locations" << std::endl; // IMPORTANT
	}
	return (server);
}

const char* EmptyValueException::what() const throw() {	return "You cannot Assign empty value"; }


////                    ******                    ////
////                PARSE LOCATIONS               ////
////                    ******                    ////


//bool validMethods(std::string &methods) {
//  std::vector<str> vMethods = split(methods, ' ');
//  RequestType req;
//  for (std::vector<str>::iterator it = vMethods.begin(); it != vMethods.end(), it++) {
//    req = strToRequest(*it);
//    if (req == -1)
//      return false;
//  }
//}



Location *getLocation(const str &locationString, const str &serverName) {
  std::istringstream locationBlock(locationString);
  std::string line;

  std::getline(locationBlock, line);
  std::string location_path = getLocationPath(line);

  std::getline(locationBlock, line);
  if (line.compare("[") != 0) { throw BadSyntaxLocationBlockException(); }

  std::string key, value;
  std::map<std::string, std::string> options;
  while (std::getline(locationBlock, line)) {
    if (!line.compare("]")) { break; }
    if (line.empty() || line.at(0) == '#') {   // Se podria sacar
      continue;                                // y pasar al parser
    }                                          // general
    std::istringstream buffer(line);
    if (std::getline(buffer, key, ':') && std::getline(buffer, value)) {
      options[key] = value;
    }
    else { throw BadSyntaxLocationBlockException(); }
  }

  Location* location = new Location(serverName, location_path);
  try {
    setLocationParams(location, options);
  } catch (BadOptionLocationException const &e) {
    delete location;
    std::cout << e.what() << std::endl;
    throw BadSyntaxLocationBlockException();
  }
  // Delete block
  std::cout << "====== Server: " << serverName << " Location path: "
            << location->getRoot() << "======" << std::endl
            << "redirect: " << location->getRedirect() << std::endl
            << "redirect_code: " << location->getRedirectCode() << std::endl
            << "uploadEnable" << ((location->getUploadEnable()) ? "true" : "false")
            << std::endl
            << "autoIndex" << ((location->getAutoindex()) ? "true" : "false")
            << std::endl
            << "index: " << location->getIndex() << std::endl
			<< "uploadPath: " << location->getUploadPath() << std::endl
			<< "cgiEnable" << ((location->getCgiEnable()) ? "true" : "false")
            << std::endl
			<< "cgiExtension: " << location->getCgiExtension() << std::endl
			<< "cgiPath: " << location->getCgiPath() << std::endl;
  // End block
  return location;
}

const char* BadOptionLocationException::what(void) const throw() {
  return "webserver: Bad option";
}

const char* BadSyntaxLocationBlockException::what(void) const throw() {
  return "webserver: configuration file failed! Bad syntax on location block";
}

