/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/26 21:35:29 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventPool.hpp"
#include "../Utils/AutoIndex.hpp"
#include "../Utils/Utils.hpp"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h> // We cant use getpid(). DELETE this line

EventPool::EventPool(std::vector<Server *> &Servers) {
  _pollFd = epoll_create(1);
  if (_pollFd == -1) {
    Logger::log("Error Creating epoll FD", ERROR);
    throw std::exception();
  }
  struct epoll_event ev;
  for (std::vector<Server *>::iterator it = Servers.begin();
       it != Servers.end(); ++it) {
    ev.events = EPOLLIN | EPOLLET;
    struct eventStructTmp *serverStruct =
        createEventStruct((*it)->getServerFd(), *it, NEWCONNECTION);
    _structs.push_back(serverStruct);
    ev.data.ptr = static_cast<void *>(serverStruct);
    if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, (*it)->getServerFd(), &ev) == -1) {
      Logger::log("Error Adding Server to epoll", ERROR);
      throw std::exception();
    }
  }
}

EventPool::~EventPool() {
  Logger::log("Destroying EventPool", USER);
  Logger::log(str("To destroy: ") + Utils::intToStr(_structs.size()) +
                  " event structs.",
              INFO);
  if (!_structs.empty())
    Utils::foreach (_structs.begin(), _structs.end(),
                    Utils::deleteItem<struct eventStructTmp>);
}

bool EventPool::getRequest(int socketFd, eventStructTmp* eventstrct) {
	char buffer[4096];
	ssize_t bytes_read = 0;
	static size_t contentLength = 0;
	static bool headerParsed = false;

	while (42) {
		bytes_read = recv(socketFd, buffer, sizeof(buffer), 0);
		if (bytes_read > 0) {
			eventstrct->content.append(buffer, bytes_read);
			eventstrct->offset += bytes_read;

			// Comprobamos si el header se vuelve demasiado grande ANTES de encontrar \r\n\r\n
			if (!headerParsed && eventstrct->content.size() > LIMIT_HEADER_SIZE)
				throw HttpException(431, "Header too large");

			if (!headerParsed) {
				size_t headerEnd = eventstrct->content.find("\r\n\r\n");
				if (headerEnd != str::npos) {
					// Ya tenemos todo el header
					headerParsed = true;
					str header = eventstrct->content.substr(0, headerEnd + 4);

					// Buscamos Content-Length
					size_t clPos = header.find("Content-Length: ");
					if (clPos != str::npos) {
						clPos += 16;
						size_t clEnd = header.find("\r\n", clPos);
						str clStr = header.substr(clPos, clEnd - clPos);
						Utils::atoi(clStr.c_str(), (int&)contentLength);

						// Comprobamos que no sea demasiado grande
						Location *loc = Utils::findLocation(eventstrct->server, "/");
						if (loc && contentLength > static_cast<size_t>(loc->getBodySize()))
							throw HttpException(413, "Payload too large (Location limit)");
						if (contentLength > eventstrct->server->getBodySize())
							throw HttpException(413, "Payload too large (Server limit)");
					}
				}
			}

			// Si ya tenemos el header y hemos leído suficiente body
			if (headerParsed) {
				size_t headerEnd = eventstrct->content.find("\r\n\r\n");
				str body = eventstrct->content.substr(headerEnd + 4);

				if (contentLength == 0 || body.size() >= contentLength) {
					headerParsed = false;
					contentLength = 0;
					return true;
				}
			}
		} else if (bytes_read == 0) {
			throw disconnectedException(socketFd);
		} else {
			break;
		}
	}
	if (eventstrct->offset == 0)
		throw disconnectedException(socketFd);

	return false; // No hemos leído todo todavía
}

void EventPool::handleClientRequest(int fd, eventStructTmp *eventStrct)
{
	try {
		if (!getRequest(fd, eventStrct))
			return;
		Logger::log(str("Received request: ") + eventStrct->content, INFO);
		HttpRequest request(eventStrct->content, eventStrct->server);
		HttpResponse response = stablishResponse(request, eventStrct->server);
		saveResponse(response, eventStrct);
	}
	catch (const HttpException& e) {
		Logger::log(e.what(), WARNING);
		saveResponse(Utils::codeResponse(e.getErrorCode(), eventStrct->server), eventStrct);
	}
	catch (const disconnectedException& e) {
		Logger::log(str("Disconnection occurred: ") + e.what(), WARNING);
		safeCloseAndDelete(fd, eventStrct);
	}
	catch (const socketReadException& e) {
		Logger::log(str("Socket Read Error: ") + e.what(), WARNING);
		saveResponse(Utils::codeResponse(400, eventStrct->server), eventStrct);
	}
	catch (...) {
		Logger::log("UNKNOWN FATAL ERROR ON handleClientRequest", ERROR);
		saveResponse(Utils::codeResponse(400, eventStrct->server), eventStrct);
	}
}

void EventPool::saveResponse(HttpResponse &response, eventStructTmp *eventStrct) {
  str resp;
  resp.append(response._line0);
  strMap::iterator it;
  for (it = response.getHeader().begin(); it != response.getHeader().end();
       ++it) {
    resp.append(it->first);
    resp.append(": ");
    resp.append(it->second);
    resp.append("\r\n");
  }
  resp.append("\r\n");
  resp.append(response.getBody());
  eventStrct->content = resp;
  eventStrct->offset = 0;
  eventStrct->eventType = SENDRESPONSE;
  struct epoll_event ev;
	ev.events = EPOLLOUT | EPOLLET;
	ev.data.ptr = static_cast<void*>(eventStrct);
  if (epoll_ctl(_pollFd, EPOLL_CTL_MOD, eventStrct->client_fd, &ev) == -1) {
		Logger::log("Failed to modify event to EPOLLOUT", ERROR);
	}
}

struct eventStructTmp *EventPool::createEventStruct(int fd, Server *server,
                                                    EventType eventType) {
  struct eventStructTmp *result = new eventStructTmp;
  result->client_fd = fd;
  result->server = server;
  result->eventType = eventType;
  result->offset = 0;
  return result;
}

void EventPool::acceptConnection(int fdTmp, Server *server) {
  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);

  int client_fd =
      accept(fdTmp, (struct sockaddr *)&client_address, &client_len);
  if (client_fd == -1) {
    Logger::log(str("No aceptada"), INFO);
    return;
  }
  else
    Logger::log(str("Conexion aceptada"), USER);
  if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
    close(client_fd);
    throw AcceptConnectionException(
        "Failed to set client with flag 0_NONBLOCK");
  }
  Logger::log(str("Accepted connection for server: ") +
                  server->getServerName() + " at port: " + server->getPort(),
              USER);
  struct epoll_event client_ev;
  client_ev.events = EPOLLIN | EPOLLET;
  struct eventStructTmp *clientStruct =
      createEventStruct(client_fd, server, RECIEVEREQUEST);
  client_ev.data.ptr = static_cast<void *>(clientStruct);
  Logger::log(str("Inserting client fd ") + Utils::intToStr(client_fd) +
                  " to pool event.",
              INFO);
  if (epoll_ctl(_pollFd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
    close(client_fd);
    throw AcceptConnectionException("Failed to add event inside epoll()!.");
  }
}

void EventPool::poolLoop() {
  while (epollRun) {
    _nfds = epoll_wait(_pollFd, events, 1024, -1);
    if (_nfds == -1) {
      if (epollRun == 0) {
        Logger::log("Graceful shutdown initiated: exiting epoll event loop due to signal.",
                    USER);
        break;
      }
      Logger::log("epoll_wait() falló", ERROR);
      perror("epoll_wait");
      throw std::exception();
    }
    Logger::log("Worker [" + Utils::intToStr(static_cast<size_t>(getpid())) + "] tooked petition", USER); // Important We cant use getpid(). DELETE this line
    processEvents();
  }
}

void EventPool::processEvents() {
  for (int i = 0; i < _nfds; ++i) {
    int fd;
    eventStructTmp *eventStrct =
        static_cast<eventStructTmp *>(events[i].data.ptr);
    if (eventStrct->eventType == NEWCONNECTION)
      fd = eventStrct->server->getServerFd();
    else
      fd = eventStrct->client_fd;
    uint32_t flags = events[i].events;
    if (flags & (EPOLLHUP | EPOLLERR)) {
      Logger::log(str("Closing fd: ") + Utils::intToStr(fd) +
                      " Because EPOLLERR or EPOLLHUP",
                  ERROR);
      safeCloseAndDelete(fd, eventStrct);
      continue ;
    }
    if (eventStrct->eventType == NEWCONNECTION) {
      handleClientConnection(fd, eventStrct);
      continue ;
    } else if (eventStrct->eventType == RECIEVEREQUEST)
      handleClientRequest(fd, eventStrct);
    else if (!handleClientWrite(fd, eventStrct))
        continue ;
  }
}

void EventPool::handleClientConnection(int fd, eventStructTmp *eventStrct) {
  try {
    acceptConnection(fd, eventStrct->server);
  } catch (AcceptConnectionException &e) {
    Logger::log(e.what(), WARNING);
  } catch (const std::exception &e) {
    Logger::log(str("FATAL UNKNOWN ERROR: ") + e.what(), ERROR);
  }
}

bool EventPool::handleClientWrite(int fd, eventStructTmp *eventStruct)
{
  Logger::log(str("Sending response: ") + eventStruct->content, INFO);
  ssize_t bytes_sent = send(fd, eventStruct->content.c_str() + eventStruct->offset,
						  eventStruct->content.size() - eventStruct->offset, 0);
  if (bytes_sent > 0) {
  	eventStruct->offset += bytes_sent;
  	if (eventStruct->offset >= eventStruct->content.size()) {
  		Logger::log("Response fully sent", USER);
  	}
  } else {
  	return false;
  }
  safeCloseAndDelete(fd, eventStruct);
  return true;
}

void EventPool::safeCloseAndDelete(int fd, eventStructTmp* eventStruct) {
	if (epoll_ctl(_pollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		Logger::log("EPOLL_CTL_DEL Failed", ERROR);
	if (eventStruct->eventType != NEWCONNECTION) {
		if (close(fd) == -1)
			Logger::log(str("Failed closing FD: ") + Utils::intToStr(fd), ERROR);
	}
	delete eventStruct; //Jamas deberia ser nulo llegado a este punto.
	eventStruct = NULL; //Protección para errores en destructor.
}

HttpResponse			EventPool::stablishResponse(HttpRequest &request, Server *server)
{
  if (request.getBadRequest())
    return Utils::codeResponse(400, server);
  else if (!request.getRedirect().empty())
    return HttpResponse(request, server);
  else if(!request.getCanAccess())
    return Utils::codeResponse(403, server);
  else if (!request.getLocation())
    return Utils::codeResponse(404, server);
  else if (!request.getValidMethod())
    return Utils::codeResponse(405, server);
  else if (!request.getIsCgi() && (request.getReceivedMethod() == "GET" && !request.getResourceExists() && !request.getLocation()->getAutoindex()))
    return Utils::codeResponse(404, server);
  else if (request.getIsCgi() && !request.getResourceExists())
    return Utils::codeResponse(404, server);
	else if (request.getLocation()->getAutoindex() && request.getReceivedMethod() == "GET" && !request.getResourceExists())
    return HttpResponse(request, server, &AutoIndex::getAutoIndex);
	else
		return HttpResponse(request, server);
}

EventPool::disconnectedException::disconnectedException(int fd) {
  this->message = str("Client Disconnected: Server = ") + Utils::intToStr(fd);
}
const char *EventPool::disconnectedException::what() const throw() {
  return this->message.c_str();
}

EventPool::socketReadException::socketReadException(int fd) {
  this->message =
      std::strerror(errno) + str(" at Server: ") + Utils::intToStr(fd);
}
const char *EventPool::socketReadException::what() const throw() {
  return this->message.c_str();
}        // checkUriSize();

EventPool::AcceptConnectionException::AcceptConnectionException(const str &msg)
    : message(msg) {}
const char *EventPool::AcceptConnectionException::what() const throw() {
  return this->message.c_str();
}

EventPool::HttpException::HttpException(int errorCode, const str& extra)
	: _errorCode(errorCode), _message(str("HTTP Exception: ") + Utils::intToStr(errorCode) + " " + extra) {}

const char* EventPool::HttpException::what() const throw() {
	return _message.c_str();
}

int EventPool::HttpException::getErrorCode() const throw() {
	return _errorCode;
}

EventPool::HttpException::~HttpException() throw() {}
