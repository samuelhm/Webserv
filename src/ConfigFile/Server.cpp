/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:47 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/05 11:20:40 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "../Utils/Logger.hpp"
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server() {
  _serverName = "server";
  _hostName = "localhost";
  _port = "8080";
  _root = "./www/html";
  _isDefault = true;
  _bodySize = 2147483647;
  _response = NULL;
  _serverFd = -1;
}

bool Server::operator==(const Server &other) {
  return (_hostName == other._hostName && _port == other._port);
}

Server::~Server() {
  Logger::log(str("Shutting down server: ") + this->_serverName, INFO);
  if (!_locations.empty())
    Utils::foreach (_locations.begin(), _locations.end(),
                    Utils::deleteItem<Location>);
  if (_response)
    freeaddrinfo(_response);
  if (_serverFd >= 0)
    close(_serverFd);
}

std::vector<Location *> &Server::getLocations() { return this->_locations; }
const str &Server::getErrorPage(int error) {
  if (_errorPages[error].empty())
    return createErrorPage(Utils::intToStr(error), Utils::_statusStr[error]);
  return _errorPages[error];
}
str Server::getServerName() const { return this->_serverName; }
str Server::getHostName() const { return this->_hostName; }
str Server::getPort() const { return this->_port; }
str Server::getRoot() const { return this->_root; }
bool Server::getIsdefault() const { return this->_isDefault; }
size_t Server::getBodySize() const { return this->_bodySize; }
int Server::getServerFd() const { return this->_serverFd; }

void Server::setLocations(std::vector<Location *> locations) {
  this->_locations = locations;
}
void Server::setErrorPages(int error, const str &page) {
  this->_errorPages[error] = page;
}
void Server::setServerName(str serverName) { this->_serverName = serverName; }
void Server::setHostName(str hostName) { this->_hostName = hostName; }
void Server::setPort(str port) { this->_port = port; }
void Server::setRoot(str root) { this->_root = root; }
void Server::setIsdefault(bool isDefault) { this->_isDefault = isDefault; }
void Server::setBodySize(size_t bodySize) { this->_bodySize = bodySize; }

void Server::socketUp() {
  Logger::log(str("Setting up server: ") + this->_serverName, USER);
  _serverFd = socket(AF_INET, SOCK_STREAM, 0);
  _reuseOption = 1;
  if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &_reuseOption,
                 sizeof(int)) == -1) {
    Logger::log(str("Fail setting SO_REUSEADDR on server: ") + _serverName,
                ERROR);
    close(_serverFd);
    throw std::exception();
  }
  if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEPORT, &_reuseOption,
                 sizeof(int)) == -1) {
    Logger::log(str("Fail setting SO_REUSEPORT on server: ") + _serverName,
                ERROR);
    close(_serverFd);
    throw std::exception();
  }
  if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1) {
    Logger::log(str("Fail setting 0_NONBLOCK on server: ") + _serverName,
                ERROR);
    close(_serverFd);
    throw std::exception();
  }
  memset(&_hints, 0, sizeof(_hints));
  _hints.ai_family = AF_INET;
  _hints.ai_socktype = SOCK_STREAM;
  _hints.ai_flags = AI_PASSIVE;
  int err = getaddrinfo(getHostName().c_str(), getPort().c_str(), &_hints,
                        &_response);
  if (err) {
    Logger::log(str("error: getaddrinfo: ") + gai_strerror(err), ERROR);
    throw std::exception();
  } else if (bind(_serverFd, _response->ai_addr, _response->ai_addrlen) == -1) {
    Logger::log(str("Cannot bind server: ") + _serverName +
                    " to port: " + _port,
                ERROR);
    throw std::exception();
  }
  if (listen(_serverFd, SOMAXCONN) == -1) {
    Logger::log(str("Listen on port: ") + _port + " Failed.", ERROR);
    close(_serverFd);
    throw std::exception();
  }
}

const str &Server::createErrorPage(const str &error, const str &msg) {
  static strMap errorHtmlMap;

  if (errorHtmlMap[error].empty()) {
    str errorHtml = "<!DOCTYPE html><html lang=\"es\"><head><meta "
                    "charset=\"UTF-8\"><meta name=\"viewport\" "
                    "content=\"width=device-width, initial-scale=1.0\"><title>";
    errorHtml.append(error + " " + msg);
    errorHtml.append(
        "</title><style>*{margin:0;padding:0;box-sizing:border-box}body{background-color:#1e1e2f;color:#fff;font-family:'Segoe UI',Tahoma,Geneva,\
						Verdana,sans-serif;display:flex;align-items:center;justify-content:center;height:100vh;text-align:center;padding:2rem}.container{max-width:600px}h1\
						{font-size:10rem;color:#ff6b6b;margin-bottom:1rem}p{font-size:1.5rem;margin-bottom:2rem}a{display:inline-block;padding:0.75rem 1.5rem;background-color:\
						#ff6b6b;color:#fff;text-decoration:none;border-radius:5px;transition:background-color 0.3s ease}a:hover{background-color:#ff4c4c}</style></head><body>\
						<div class=\"container\"><h1>");
    errorHtml.append(error + "</h1><p>");
    errorHtml.append(
        msg + "</p><a href=\"/\">Volver al inicio</a></div></body></html>");
    errorHtmlMap[error] = errorHtml;
  }
  return errorHtmlMap[error];
}

bool Server::locationExist(Location &loc) const {
  for (size_t i = 0; i < _locations.size(); i++)
    if (_locations[i]->getUrlPath() == loc.getUrlPath())
      return true;
  return false;
}

void Server::setListenValue(const str &value) {
  size_t sep = value.find(":");
  if (sep == str::npos)
    throw ConfigFileException("LISTEN must be in format hostname:port " +
                              value);
  str hostname = Utils::trim(value.substr(0, sep));
  str port = Utils::trim(value.substr(sep + 1));
  if (hostname.empty() || port.empty())
    throw ConfigFileException("LISTEN has empty host or port: " + value);
  this->setHostName(hostname);
  this->setPort(port);
}
