/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/30 16:13:47 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventPool.hpp"
#include <stdio.h>

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
		ev.data.fd = (*it)->getServerFd();
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
	std::cout << buffer << std::endl;
	return (buffer);
}

void	EventPool::sendResponse(HttpResponse &response, int fdTmp)
{
	const char* http_response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 45\r\n"
				"\r\n"
				"<html><body><h1>Hola mundo</h1></body></html>";

	ssize_t error = write(fdTmp, http_response, strlen(http_response));
	if (error == -1)
		perror("write");
	epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL);
	close(fdTmp);
}

void	EventPool::acceptConnection(int fdTmp)
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
	client_ev.data.fd = client_fd;
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
	while (1)
	{
		_nfds = epoll_wait(_pollFd, events, 1024, -1); // -1 = bloquea indefinidamente
		if (_nfds == -1) {
			perror("epoll_wait");
			throw std::exception();
		}
		for (int i = 0; i < _nfds; ++i)
		{
			int fdTmp = events[i].data.fd;

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
					acceptConnection(fdTmp);
				} catch(const std::exception& e) {
					std::cout << e.what() << std::endl;
					continue;
				}
			}
			else
			{
				try {
					HttpRequest request(getRequest(fdTmp));
					HttpResponse response(request);
					sendResponse(response, fdTmp);
				} catch(const disconnectedException& e) {
					std::cout << e.what() << std::endl;
					continue;
				} catch(const socketReadException& e) {
					std::cout << e.what() << std::endl;
					continue;
				} catch(...)
				{
					std::cout << "Error no manejado" << std::endl;
				}
			}
		}
	}
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
