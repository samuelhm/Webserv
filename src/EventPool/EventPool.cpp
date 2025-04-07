/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/07 11:22:13 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventPool.hpp"
#include <stdio.h>
#include <stdlib.h>

EventPool::EventPool(std::vector<Server*> &Servers) {
	_pollFd = epoll_create(1);
	if (_pollFd == -1) {
		std::cout << "error: epoll_create1" << std::endl;
		throw std::exception();
	}

	struct epoll_event ev;

	for (std::vector<Server*>::iterator it = Servers.begin(); it != Servers.end(); ++it)
	{
		ev.events = EPOLLIN;
		// ev.data.fd = (*it)->getServerFd();
		struct eventStructtmp* estructura = new eventStructtmp; // IMPORTANT free this
		estructura->server = *it;
		estructura->isServer = true;
		ev.data.ptr = static_cast<void*>(estructura);
		if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, (*it)->getServerFd(), &ev) == -1) {
			perror("epoll_ctl");
			throw std::exception();
		}
	}
}

EventPool::EventPool(const EventPool &other) {
	// Constructor de copia
	*this = other;
}

EventPool& EventPool::operator=(const EventPool &other) {
	// Operador de asignación
	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

EventPool::~EventPool() {
	// Destructor
}

str		EventPool::getRequest(int fdTmp)
{
	char buffer[4096]; //comprobar si es suficiente ( caso de post de archivos )
	ssize_t bytes_read = read(fdTmp, buffer, sizeof(buffer) - 1);
	if (bytes_read <= 0) {
		if (bytes_read == 0)
			throw disconnectedException(fdTmp);
		throw socketReadException(fdTmp);
	}
	buffer[bytes_read] = '\0';

	str request(buffer);
	size_t pos = request.find("\r\n");
	str first_line;
	if (pos != std::string::npos)
		first_line = request.substr(0, pos);
	Logger::log(str("HTTP Request Received.") + buffer, INFO);
	Logger::log(str("HTTP Request Received.") + first_line, USER);
	return (buffer);
}

void	EventPool::sendResponse(HttpResponse &response, int fdTmp, const std::map<str, str>& m)
{
	(void)response; //IMPORTANT delete
	(void)m;
	str err;
	err.append(response._line0);
	std::map<str, str>::iterator it;
	for (it = response.get_header().begin(); it != response.get_header().end(); ++it) {
		err.append(it->first);
		err.append(": ");
		err.append(it->second);
		err.append("\r\n");
	}
	err.append(response.get_body());
	ssize_t error = write(fdTmp, err.c_str(), err.size());
	if (error == -1) {
		perror("write");
		Logger::log("Cannot Write on Socket!.", ERROR);
	}
	epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL);
	close(fdTmp);
	Logger::log("Sending resource.", INFO);
}

void	EventPool::acceptConnection(int fdTmp, Server* server)
{
	struct sockaddr_in client_address;
	socklen_t client_len = sizeof(client_address);

	int client_fd = accept(fdTmp, (struct sockaddr *)&client_address, &client_len);
	if (client_fd == -1)
		throw std::exception();
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("fcntl failed");
		close(client_fd);
		throw std::exception();
	}
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	struct eventStructtmp* estructura = new eventStructtmp; // IMPORTANT free this
	estructura->client_fd = client_fd;
	estructura->server = server;
	estructura->isServer = false;
	client_ev.data.ptr = static_cast<void*>(estructura);
    if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
        perror("epoll_ctl failed");
        close(client_fd);
		throw std::exception();
    }
}

bool EventPool::isServerFd(std::vector<Server *> &Servers, int fdTmp)
{
	for (size_t s = 0; s < Servers.size(); ++s)
	{
		if (fdTmp == Servers[s]->getServerFd())
			return true;
	}
	return false;
}

void	EventPool::poolLoop(std::vector<Server*> &Servers)
{
	while (42)
	{
		int fdTmp;
		_nfds = epoll_wait(_pollFd, events, 1024, -1); // -1 = bloquea indefinidamente
		if (_nfds == -1) {
			perror("epoll_wait");
			throw std::exception();
		}
		for (int i = 0; i < _nfds; ++i)
		{
			if (static_cast<eventStructtmp *>(events[i].data.ptr)->isServer)
				fdTmp =  static_cast<eventStructtmp *>(events[i].data.ptr)->server->getServerFd();
			else
				fdTmp =  static_cast<eventStructtmp *>(events[i].data.ptr)->client_fd;
			uint32_t flags = events[i].events;
			if (flags & (EPOLLHUP | EPOLLERR)) {
				std::cout << "Cerrando conexión con fd: " << fdTmp << std::endl;
				if (epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL) == -1) {
					perror("epoll_ctl failed");
				}
				close(fdTmp);
				continue;
			}
			if (isServerFd(Servers, fdTmp))
			{
				try {
					acceptConnection(fdTmp, static_cast<eventStructtmp *>(events[i].data.ptr)->server);
				} catch(const std::exception& e) {
					Logger::log(str("acceptConnection Error: ") + e.what(), ERROR);
					continue;
				}
			}
			else
			{
				try {
					str reqStr = getRequest(fdTmp);
					HttpRequest request(reqStr);
					HttpResponse response(request, static_cast<eventStructtmp *>(events[i].data.ptr)->server);
					sendResponse(response, fdTmp, response.get_header());
				} catch(const disconnectedException& e) {
					Logger::log(str("Disconnection: ") + e.what(), WARNING);
				} catch(const socketReadException& e) {
					Logger::log(str("Socket read Error: ") + e.what(), ERROR);
				} catch(...) {
					Logger::log("Unknown Erro on epoll: ", ERROR);
				}
			}
		}
	}
}


Server* EventPool::getServerByFd(int fd, std::vector<Server*> Servers)
{
	std::vector<Server*>::iterator it;
	for (it = Servers.begin(); it != Servers.end(); ++it)
	{
		std::cout << "fd que estamos buscando: " << fd << std::endl;
		std::cout << "fd que estamos Iterando: " << (*it)->getServerFd() << std::endl;
		if ((*it)->getServerFd() == fd)
			return (*it);
	}
	std::cout << "Devolviendo NULL" << std::endl;
	return NULL;
}


EventPool::disconnectedException::disconnectedException(int fd) {
	std::ostringstream oss;
	oss << fd;
	this->message = "Client Disconnected: Server = " + oss.str(); //Deve obtener el nombre del servidor
}
const char *EventPool::disconnectedException::what() const throw() { return this->message.c_str(); }

EventPool::socketReadException::socketReadException(int fd) {
	std::ostringstream oss;
	oss << fd;
	this->message = std::strerror(errno) + std::string(" at Server: ") + oss.str();
}
const char *EventPool::socketReadException::what() const throw() { return this->message.c_str(); }
