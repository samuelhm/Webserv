/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:47 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 12:19:00 by shurtado         ###   ########.fr       */
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
	_isdefault = true;
	_bodySize = 1048576;
	_cgiEnable = false;
	_cgiExtension = ".py";
	_cgiPath = "/usr/bin/python3";
}

Server::Server(const Server &other) {
	// Constructor de copia
	*this = other;
}

Server& Server::operator=(const Server &other) {
	// Operador de asignación
	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

Server::~Server() {
	// Destructor
}


std::string	Server::getHostName() const {
	return _hostName;
}
int			Server::getPort() const {
	return _port;
}
