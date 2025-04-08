/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 01:26:24 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventPool.hpp"
#include "../Utils/Utils.hpp"
#include <stdio.h>
#include <stdlib.h>

EventPool::EventPool(std::vector<Server*> &Servers) {
	_pollFd = epoll_create(1);
	if (_pollFd == -1) {
		Logger::log("Error Creating epoll FD", ERROR);
		throw std::exception();
	}
	struct epoll_event ev;
	for (std::vector<Server*>::iterator it = Servers.begin(); it != Servers.end(); ++it)
	{
		ev.events = EPOLLIN;
		struct eventStructTmp* serverStruct = createEventStruct((*it)->getServerFd(), *it, true);
		_structs.push_back(serverStruct);
		ev.data.ptr = static_cast<void*>(serverStruct);
		if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, (*it)->getServerFd(), &ev) == -1) {
			Logger::log("Error Adding Server to epoll", ERROR);
			throw std::exception();
		}
	}
}

EventPool& EventPool::operator=(const EventPool &other) {
	// Operador de asignación
	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

EventPool::~EventPool() {
	if (!_structs.empty())
		Utils::foreach(_structs.begin(), _structs.end(), Utils::deleteItem<struct eventStructTmp>);
}

str		EventPool::getRequest(int fdTmp)
{
	char buffer[4096]; //IMPORTANT Considerar un bucle de lectura por bloques si bytes_read == sizeof(buffer) - 1
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
	if (!first_line.empty())
		Logger::log(str("HTTP Request Received.") + first_line, USER);
	else
		Logger::log("no \\r\\n found!!!", USER);
	return (buffer);
}

void	EventPool::sendResponse(HttpResponse &response, int fdTmp, const std::map<str, str>& m)
{
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
	Logger::log(str("Sending resource to fd: ") + Utils::intToStr(fdTmp), USER);
	Logger::log(str("Sending to client the header: \n") + Utils::returnMap(m), INFO);
	Logger::log(str("Sending to client the body:\n") + response.get_body(), INFO);
}

struct eventStructTmp* EventPool::createEventStruct(int fd, Server* server, bool serverOrClient)
{
	struct eventStructTmp* result = new eventStructTmp;
	result->client_fd = fd;
	result->server = server;
	result->isServer = serverOrClient;
	return result;
}

void	EventPool::acceptConnection(int fdTmp, Server* server)
{
	struct sockaddr_in client_address;
	socklen_t client_len = sizeof(client_address);

	int client_fd = accept(fdTmp, (struct sockaddr *)&client_address, &client_len);
	if (client_fd == -1)
		throw AcceptConnectionException("Failed on Accept connetion for server: " + server->getServerName());
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
		close(client_fd);
		throw AcceptConnectionException("Failed to set client with flag 0_NONBLOCK");
	}
	Logger::log(str("Accepted connection for server: ") + server->getServerName() + " at port: " + server->getPort(), USER);
	struct epoll_event client_ev;
	client_ev.events = EPOLLIN;
	struct eventStructTmp* clientStruct = this->createEventStruct(client_fd, server, false);
	client_ev.data.ptr = static_cast<void*>(clientStruct);
	Logger::log(str("Inserting client fd ") + Utils::intToStr(client_fd) + " to pool event.", INFO);
    if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
        close(client_fd);
		throw AcceptConnectionException("Failed to add event inside epoll()!.");
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
	while (epollRun)
	{
		int fdTmp;
		_nfds = epoll_wait(_pollFd, events, 1024, -1); // -1 = bloquea indefinidamente
		if (_nfds == -1) {
			if (errno == EINTR) { //IMPORTANT para explicar: También se podría: if (epollRun == 0) vendría siendo lo mismo
				Logger::log("Graceful shutdown initiated: exiting epoll event loop due to signal.", USER);
				break;
			}
			Logger::log("epoll_wait() falló", ERROR);
			perror("epoll_wait");
			throw std::exception();
		}
		for (int i = 0; i < _nfds; ++i)
		{
			if (static_cast<eventStructTmp *>(events[i].data.ptr)->isServer)
				fdTmp =  static_cast<eventStructTmp *>(events[i].data.ptr)->server->getServerFd();
			else
				fdTmp =  static_cast<eventStructTmp *>(events[i].data.ptr)->client_fd;
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
					acceptConnection(fdTmp, static_cast<eventStructTmp *>(events[i].data.ptr)->server);
				} catch (AcceptConnectionException &e) {
					Logger::log(e.what(), WARNING);
				}
				catch(const std::exception& e) {
					Logger::log(str("FATAL UNKNOWN ERROR: ") + e.what(), ERROR);
					continue;
				}
			}
			else
			{
				try {
					str reqStr = getRequest(fdTmp);
					HttpRequest request(reqStr);
					HttpResponse response(request, static_cast<eventStructTmp *>(events[i].data.ptr)->server);
					sendResponse(response, fdTmp, response.get_header());
					delete static_cast<eventStructTmp *>(events[i].data.ptr);
				} catch(const disconnectedException& e) {
					Logger::log(str("Disconnection occur: ") + e.what(), WARNING);  //IMPORTANT Meter todo lo repetitivo en una funcion, y proteger epoll_ctl ante errores (o no).
					epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL);
					if (!static_cast<eventStructTmp *> (events[i].data.ptr)->isServer)
						close (fdTmp);
					delete static_cast<eventStructTmp *>(events[i].data.ptr);
				} catch(const socketReadException& e) {
					Logger::log(str("Socket read Error: ") + e.what(), ERROR);
					epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL);
					if (!static_cast<eventStructTmp *> (events[i].data.ptr)->isServer)
					close (fdTmp);
					delete static_cast<eventStructTmp *>(events[i].data.ptr);
				} catch(...) {
					Logger::log("Unknown Error on epoll: ", ERROR);
					epoll_ctl(_pollFd, EPOLL_CTL_DEL, fdTmp, NULL);
					if (!static_cast<eventStructTmp *> (events[i].data.ptr)->isServer)
					close (fdTmp);
					delete static_cast<eventStructTmp *>(events[i].data.ptr);
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

EventPool::AcceptConnectionException::AcceptConnectionException(const str &msg) : message(msg) {}
const char *EventPool::AcceptConnectionException::what() const throw() { return this->message.c_str(); }
