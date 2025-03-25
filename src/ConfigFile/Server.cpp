/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:47 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 16:26:38 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Server.hpp"

Server::Server() {

	_serverName = "server1";
	Location location(_serverName);
	_locations.push_back(location);
	_hostName = "localhost";
	_port = 8080;
	_root =	"./www/html";
	_isDefault = true;
	_bodySize = 1048576;
	_cgiEnable = false;
	_cgiExtension = ".py";
	_cgiPath = "/usr/bin/python3";
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
		this->_cgiEnable = other._cgiEnable;
		this->_cgiExtension = other._cgiExtension;
		this->_cgiPath = other._cgiPath;
	}
	return *this;
}

Server::~Server() {}

//Getters
std::vector<Location>		Server::getLocations() const { return this->_locations; }
std::map<int, std::string>	Server::getErrorPages() const { return this->_errorPages; }
std::string					Server::getServerName() const { return this->_serverName; }
std::string					Server::getHostName() const { return this->_hostName; }
int							Server::getPort() const { return this->_port; }
std::string					Server::getRoot() const { return this->_root; }
bool						Server::getIsdefault() const { return this->_isDefault; }
size_t						Server::getBodySize() const { return this->_bodySize; }
bool						Server::getCgiEnable() const { return this->_cgiEnable; }
std::string					Server::getCgiExtension() const { return this->_cgiExtension; }
std::string					Server::getCgiPath() const { return this->_cgiPath; }

//Setters
void						Server::setLocations(std::vector<Location> locations) {this->_locations = locations;}
void						Server::setErrorPages(std::map<int, std::string> errorPages) {this->_errorPages = errorPages;}
void						Server::setServerName(std::string serverName) {this->_serverName = serverName;}
void						Server::setHostName(std::string hostName) {this->_hostName = hostName;}
void						Server::setPort(int port) {this->_port = port;}
void						Server::setRoot(std::string root) {this->_root = root;}
void						Server::setIsdefault(bool isDefault) {this->_isDefault = isDefault;}
void						Server::setBodySize(size_t bodySize) {this->_bodySize = bodySize;}
void						Server::setCgiEnable(bool cgiEnable) {this->_cgiEnable = cgiEnable;}
void						Server::setCgiExtension(std::string cgiExtension) {this->_cgiExtension = cgiExtension;}
void						Server::setCgiPath(std::string cgiPath) {this->_cgiPath = cgiPath;}
