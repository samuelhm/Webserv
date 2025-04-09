/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:47 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 10:15:27 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <stdlib.h>

Server::Server() {
	_serverName = "server1";
	_hostName = "localhost";
	_port = "8080";
	_root =	"./www/html";
	_isDefault = true;
	_bodySize = 2147483647;
}

bool	Server::operator==(const Server &other)
{
	return (_hostName == other._hostName && _port == other._port);
}

Server::~Server() {
	Logger::log(str("Shutting down server: ") + this->_serverName, INFO);
	if (!_locations.empty())
		Utils::foreach(_locations.begin(), _locations.end(), Utils::deleteItem<Location>);
	if (_response)
		freeaddrinfo(_response);
	close(_serverFd);
}

//Getters
std::vector<Location*>&		Server::getLocations() { return this->_locations; } //IMPORTANT es getter pero no puede ser const y debe devolver referencia para poder hacer push_back, hacer otro?
const str&					Server::getErrorPage(int error)
{
	if (_errorPages[error].empty())
		return createErrorPage(Utils::intToStr(error), Utils::_statusStr[error]);
	return _errorPages[error];
}
str							Server::getServerName() const { return this->_serverName; }
str							Server::getHostName() const { return this->_hostName; }
str							Server::getPort() const { return this->_port; }
str							Server::getRoot() const { return this->_root; }
bool						Server::getIsdefault() const { return this->_isDefault; }
size_t						Server::getBodySize() const { return this->_bodySize; }
int							Server::getServerFd() const { return this->_serverFd; }

//Setters
void						Server::setLocations(std::vector<Location*> locations) { this->_locations = locations; }
void						Server::setErrorPages(int error, const str &page) { this->_errorPages[error] = page; }
void						Server::setServerName(str serverName) { this->_serverName = serverName; }
void						Server::setHostName(str hostName) { this->_hostName = hostName; }
void						Server::setPort(str port) { this->_port = port; }
void						Server::setRoot(str root) { this->_root = root; }
void						Server::setIsdefault(bool isDefault) { this->_isDefault = isDefault; }
void						Server::setBodySize(size_t bodySize) { this->_bodySize = bodySize; }

void						Server::socketUp()
{
	Logger::log(str("Setting up server: ") + this->_serverName, USER);
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	_reuseOption = 1;
// Poder reutilizar el mismo puerto sin tener que esperar en caso de fallo del programa
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &_reuseOption, sizeof(int)) == -1) {
		perror("setsockopt failed");
		close(_serverFd);
		throw std::exception();
	}
	if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl failed");
		close(_serverFd);
		throw std::exception();
	}
// carga el host name y port en el addrinfo
// hints -> asignamos opciones de config
	memset(&_hints, 0, sizeof(_hints));
	_hints.ai_family = AF_INET;			// IPv4
	_hints.ai_socktype = SOCK_STREAM;	// TCP
	_hints.ai_flags = AI_PASSIVE;		// Para bind() (escuchar)
// con las opciones dadas en hints configura res
	int err = getaddrinfo(getHostName().c_str(), getPort().c_str(), &_hints, &_response);
	if (err) {
		std::cout << "error: getaddrinfo" << std::endl << gai_strerror(err) << std::endl;
		return ;
	}
// vincula fd del servidor al host name y port
	else if (bind(_serverFd, _response->ai_addr, _response->ai_addrlen) == -1) {
		std::cout << "error: bind" << std::endl;
		return ;
	}
// habilitamos el fd para que se quede escuchando
	if (listen(_serverFd, SOMAXCONN) == -1) {
		perror("listen failed");
		close(_serverFd);
		throw std::exception();
	}
}

const str &Server::createErrorPage(const str &error, const str &msg)
{
	static std::map<str, str> errorHtmlMap;

	if (errorHtmlMap[error].empty()) {
		str errorHtml ="<!DOCTYPE html><html lang=\"es\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>";
		errorHtml.append(error + " " + msg);
		errorHtml.append("</title><style>*{margin:0;padding:0;box-sizing:border-box}body{background-color:#1e1e2f;color:#fff;font-family:'Segoe UI',Tahoma,Geneva,\
						Verdana,sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;text-align:center;padding:2rem}.container{max-width:600px}h1\
						{font-size:10rem;color:#ff6b6b;margin-bottom:1rem}p{font-size:1.5rem;margin-bottom:2rem}a{display:inline-block;padding:0.75rem 1.5rem;background-color:\
						#ff6b6b;color:#fff;text-decoration:none;border-radius:5px;transition:background-color 0.3s ease}a:hover{background-color:#ff4c4c}</style></head><body>\
						<div class=\"container\"><h1>");
		errorHtml.append(error + "</h1><p>");
		errorHtml.append(msg + "</p><a href=\"/\">Volver al inicio</a></div></body></html>");
		errorHtmlMap[error] = errorHtml;
	}
	return errorHtmlMap[error];
}

bool	Server::locationExist(Location &loc) const
{
	for(size_t i = 0; i < _locations.size(); i++)
		if (_locations[i]->getRoot() == loc.getRoot())
			return true;
	return false;
}
