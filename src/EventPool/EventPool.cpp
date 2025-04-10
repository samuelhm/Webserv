/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 11:27:19 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventPool.hpp"
#include "../Utils/Utils.hpp"
#include "../Utils/AutoIndex.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

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

EventPool::~EventPool() {
	Logger::log("Destroying EventPool", USER);
	Logger::log(str("To destroy: ") + Utils::intToStr(_structs.size()) + " event structs.", INFO);
	if (!_structs.empty())
		Utils::foreach(_structs.begin(), _structs.end(), Utils::deleteItem<struct eventStructTmp>);
}

str		EventPool::getRequest(int fdTmp)
{
	str request;
	char buffer[4096]; //IMPORTANT Considerar un bucle de lectura por bloques si bytes_read == sizeof(buffer) - 1
	std::memset(buffer, 0, 4096);
	ssize_t total_bytes = 0;
	ssize_t bytes_read = read(fdTmp, buffer, sizeof(buffer) - 1);
	while (bytes_read > 0)
	{
		total_bytes += bytes_read;
		request.append(buffer);
		bytes_read = (read(fdTmp, buffer, sizeof(buffer) - 1) > 0);
	}
	if (total_bytes <= 0) {
		if (total_bytes == 0)
			throw disconnectedException(fdTmp);
		throw socketReadException(fdTmp);
	}
	size_t pos = request.find("\r\n");
	str first_line;
	if (pos != std::string::npos)
		first_line = request.substr(0, pos);
	Logger::log(str("HTTP Request Received.") + request, INFO);
	if (!first_line.empty())
		Logger::log(str("HTTP Request Received.") + first_line, USER);
	else
		Logger::log("no \\r\\n found!!!", USER);
	return (request);
}

void	EventPool::sendResponse(HttpResponse &response, int fdTmp, const strMap& m)
{
	str err;
	err.append(response._line0);
	strMap::iterator it;
	for (it = response.get_header().begin(); it != response.get_header().end(); ++it) {
		err.append(it->first);
		err.append(": ");
		err.append(it->second);
		err.append("\r\n");
	}
	err.append(response.get_body());
	ssize_t error = write(fdTmp, err.c_str(), err.size());
	if (error == -1)
		Logger::log("Cannot Write on Socket!.", ERROR);
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
	struct eventStructTmp* clientStruct = createEventStruct(client_fd, server, false);
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
		processEvents(Servers);
	}
}

void	EventPool::processEvents(std::vector<Server*> &Servers)
{
	for (int i = 0; i < _nfds; ++i)
	{
		int fd;
		eventStructTmp *eventStrct = static_cast<eventStructTmp *>(events[i].data.ptr);
		if (eventStrct->isServer)
			fd =  eventStrct->server->getServerFd();
		else
			fd =  eventStrct->client_fd;
		uint32_t flags = events[i].events;
		if (flags & (EPOLLHUP | EPOLLERR)) {
			Logger::log(str("Closing fd: ")+ Utils::intToStr(fd) + " Because EPOLLERR or EPOLLHUP", ERROR);
			safeCloseAndDelete(fd, eventStrct);
			continue;
		}
		if (isServerFd(Servers, fd)) {
			handleClientConnection(fd, eventStrct);
			continue;
		}
		else
			handleClientRequest(fd, eventStrct);
	}
}

void	EventPool::handleClientConnection(int fd, eventStructTmp *eventStrct)
{
	try {
		acceptConnection(fd, eventStrct->server); }
	catch (AcceptConnectionException &e) {
		Logger::log(e.what(), WARNING); }
	catch(const std::exception& e) {
		Logger::log(str("FATAL UNKNOWN ERROR: ") + e.what(), ERROR); }
}

bool	EventPool::checkCGI(str path, Server server)
{
	// if (!isCgi(path))
		// return false;
	str Path = path;
	str ext = path.substr(path.find_last_of('.'));
	Path.append("/");
	Path = AutoIndex::getPrevPath(Path);
	Path.erase(Path.size() - 1);
	std::vector<Location*> locations = server.getLocations();
	for (int i = 0; i < locations.size(); i++)
	{
		if (Path == locations[i]->getRoot())
		{
			if (!locations[i]->getCgiEnable())
				return false;
			std::vector<str> extensions = locations[i]->getCgiExtension();
			for (int j = 0; j < extensions.size(); j++)
			{
				if (ext == extensions[j])
					return true;
			}
			return false;
		}
	}
	return false;
}

void	EventPool::handleClientRequest(int fd, eventStructTmp *eventStrct)
{
	try {
		str reqStr = getRequest(fd);
		HttpRequest request(reqStr, eventStrct->server);
		HttpResponse response(request, eventStrct->server);
		sendResponse(response, fd, response.get_header());
	} catch(const disconnectedException& e) {
		Logger::log(str("Disconnection occur: ") + e.what(), WARNING);
	} catch(const socketReadException& e) {
		Logger::log(str("Socket Read Error: ") + e.what(), WARNING);
	} catch(...) {
		Logger::log("UNKNOWN FATAL ERROR ON handleClientRequest", ERROR);
	}
	safeCloseAndDelete(fd, eventStrct);
}

void EventPool::safeCloseAndDelete(int fd, eventStructTmp* eventStruct) {
	if (epoll_ctl(_pollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		Logger::log("EPOLL_CTL_DEL Failed", ERROR);
	if (!eventStruct->isServer) {
		if (close(fd) == -1)
			Logger::log(str("Failed closing FD: ") + Utils::intToStr(fd), ERROR);
	}
	delete eventStruct; //Jamas deberia ser nulo llegado a este punto.
	eventStruct = NULL; //Protección para errores en destructor.
}

EventPool::disconnectedException::disconnectedException(int fd) {
	this->message = str("Client Disconnected: Server = ") + Utils::intToStr(fd);
}
const char *EventPool::disconnectedException::what() const throw() { return this->message.c_str(); }

EventPool::socketReadException::socketReadException(int fd) {
	this->message = std::strerror(errno) + std::string(" at Server: ") + Utils::intToStr(fd);
}
const char *EventPool::socketReadException::what() const throw() { return this->message.c_str(); }

EventPool::AcceptConnectionException::AcceptConnectionException(const str &msg) : message(msg) {}
const char *EventPool::AcceptConnectionException::what() const throw() { return this->message.c_str(); }
