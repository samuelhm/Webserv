/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/20 19:51:51 by shurtado         ###   ########.fr       */
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

bool EventPool::headerTooLarge(str const &request, int &errorCode) {
  bool result = false;
	size_t end = request.find("\r\n\r\n");
	if (end == std::string::npos)
		result = (request.size() > LIMIT_HEADER_SIZE);
	else
    result = (end > LIMIT_HEADER_SIZE);
  if (result)
    errorCode = 431;
  return !result;
}

bool  EventPool::checkBodySize(eventStructTmp* eventstrct, int &errorCode) {
  size_t first_space = eventstrct->content.find(' ');
	size_t second_space = eventstrct->content.find(' ', first_space + 1);
	const str uri = eventstrct->content.substr(first_space + 1, second_space - first_space - 1);
  Location *loc = Utils::findLocation(eventstrct->server, uri);
  const str endhd = "\r\n\r\n";
  const size_t endpos = eventstrct->content.find(endhd);
  const str body = eventstrct->content.substr(endpos + endhd.size());
  const str clfind = "Content-Length: ";
  const str header = eventstrct->content.substr(0, endpos + endhd.size());
  const size_t clfindEnd = header.find(clfind) + clfind.size();

  str contentLength = header.substr(clfindEnd, header.find("\r\n", clfindEnd) - clfindEnd);
  int length;
  Utils::atoi(contentLength.c_str(), length);
  size_t clength = static_cast<size_t>(length);
  if (clength > body.size() || length > loc->getBodySize())
  {
    if (length > loc->getBodySize())
      errorCode = 413;
    return true;
  }
  return false;
}

bool EventPool::getRequest(int socketFd, eventStructTmp* eventstrct) {
  char buffer[4096];
  ssize_t   bytes_read = 0;

  while (42) {
	  bytes_read = recv(socketFd, buffer, sizeof(buffer), 0);
	  if (bytes_read > 0) {
	  	eventstrct->content.append(buffer, bytes_read);
	  	eventstrct->offset += bytes_read;
      if (eventstrct->content.find("\r\n\r\n") == str::npos)
        return false;
    }
    else if (bytes_read == 0)
      throw disconnectedException(socketFd);
    else
      break;
  }
  if (eventstrct->offset == 0)
    throw disconnectedException(socketFd);
  Logger::log(str("HTTP Request Received.") + eventstrct->content, USER);
  return true;
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

bool EventPool::isServerFd(std::vector<Server *> &Servers, int fdTmp) {
  for (size_t s = 0; s < Servers.size(); ++s) {
    if (fdTmp == Servers[s]->getServerFd())
      return true;
  }
  return false;
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

void	EventPool::handleClientRequest(int fd, eventStructTmp *eventStrct)
{
  int errorCode = 400;
	try {
    if (!getRequest(fd, eventStrct))
    {
      if (headerTooLarge(eventStrct->content, errorCode) || checkBodySize(eventStrct, errorCode))
        return ;
      else
        saveResponse(Utils::codeResponse(errorCode, eventStrct->server), eventStrct);
    }
    else {
      Logger::log(str("Received request: ") + eventStrct->content, INFO);
		  HttpRequest request(eventStrct->content, eventStrct->server);
		  HttpResponse response = stablishResponse(request, eventStrct->server);
		  saveResponse(response, eventStrct);
    }
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

EventPool::headerTooLargeException::headerTooLargeException(int fd)
	: message(str("Header too large at socket: ") + Utils::intToStr(fd))
{}

const char *EventPool::headerTooLargeException::what() const throw() {
  return message.c_str();
}
