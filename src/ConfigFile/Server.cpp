/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:47 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/07 17:58:55 by shurtado         ###   ########.fr       */
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
	_errorPages[400] = createErrorPage("400", "Bad Request");
	_errorPages[401] = createErrorPage("401", "Unauthorized");
	_errorPages[402] = createErrorPage("402", "Payment Required");
	_errorPages[403] = createErrorPage("403", "Forbidden");
	_errorPages[404] = createErrorPage("404", "Not Found");
	_errorPages[405] = createErrorPage("405", "Method Not Allowed");
	_errorPages[406] = createErrorPage("406", "Not Acceptable");
	_errorPages[407] = createErrorPage("407", "Proxy Authentication Required");
	_errorPages[408] = createErrorPage("408", "Request Timeout");
	_errorPages[409] = createErrorPage("409", "Conflict");
	_errorPages[410] = createErrorPage("410", "Gone");
	_errorPages[411] = createErrorPage("411", "Length Required");
	_errorPages[412] = createErrorPage("412", "Precondition Failed");
	_errorPages[413] = createErrorPage("413", "Content Too Large");
	_errorPages[414] = createErrorPage("414", "URI Too Long");
	_errorPages[415] = createErrorPage("415", "Unsupported Media Type");
	_errorPages[416] = createErrorPage("416", "Range Not Satisfiable");
	_errorPages[417] = createErrorPage("417", "Expectation Failed");
	_errorPages[421] = createErrorPage("421", "Misdirected Request");
	_errorPages[422] = createErrorPage("422", "Unprocessable Content");
	_errorPages[426] = createErrorPage("426", "Upgrade Required");
	_errorPages[500] = createErrorPage("500", "Internal Server Error");
	_errorPages[501] = createErrorPage("501", "Not Implemented");
	_errorPages[502] = createErrorPage("502", "Bad Gateway");
	_errorPages[503] = createErrorPage("503", "Service Unavailable");
	_errorPages[504] = createErrorPage("504", "Gateway Timeout");
	_errorPages[505] = createErrorPage("505", "HTTP Version Not Supported");

	socketUp();
}

Server::Server(const str &servername) {

	_serverName = servername;
	_hostName = "localhost";
	_port = "8080";
	_root =	"./www/html";
	_isDefault = true;
	_bodySize = 1048576;
	socketUp();
}


Server::Server(const str &servername, const str &port) {

	_serverName = servername;
	_hostName = "localhost";
	_port = port;
	_root =	"./www/html";
	_isDefault = true;
	_bodySize = 1048576;
	socketUp();
}

Server::Server(const Server &other) { *this = other; }

Server& Server::operator=(const Server &other) {
	if (this != &other) {
		this->_locations = other._locations;
		this->_errorPages = other._errorPages;
		this->_serverName = other._serverName;
		this->_hostName = other._hostName;
		this->_port = other._port;
		this->_root = other._root;
		this->_isDefault = other._isDefault;
		this->_bodySize = other._bodySize;
	}
	return *this;
}

Server::~Server() {
	if (!_locations.empty())
		Utils::foreach(_locations.begin(), _locations.end(), Utils::deleteItem<Location>);
	if (_response)
		freeaddrinfo(_response);
	close(_serverFd);
}

//Getters
std::vector<Location*>&		Server::getLocations() { return this->_locations; } //IMPORTANT es getter pero no puede ser const y debe devolver referencia para poder hacer push_back, hacer otro?
const str&					Server::getErrorPage(int error) { return _errorPages[error]; }
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

	int err = std::atoi(error);
	if ((err <= 400 && err >= 426) && (err <= 500 && err >= 505)) {
		Logger::log("createErrorPage ha recibido un error incorrecto", ERROR);
		return "ERROR";
	}
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
