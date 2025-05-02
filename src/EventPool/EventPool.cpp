/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:03 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/02 20:20:22 by shurtado         ###   ########.fr       */
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

void  EventPool::checkBodySize(eventStructTmp* eventstrct)
{
  size_t headerEnd = eventstrct->content.find("\r\n\r\n");
  std::string body = eventstrct->content.substr(headerEnd + 4);
  if (body.size() > eventstrct->server->getBodySize()) {
      Logger::log("Payload too large (Content-Length body)", WARNING);
      throw HttpException(413, "Payload Too Large");
  }
}

void	EventPool::parseHeader(eventStructTmp* eventstrct)
{
    size_t headerEnd = eventstrct->content.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        eventstrct->headerParsed = true;
        std::string header = eventstrct->content.substr(0, headerEnd + 4);
        if (header.find("Transfer-Encoding: chunked") != std::string::npos) {
            eventstrct->isChunked = true;
        } else {
            size_t clPos = header.find("Content-Length: ");
            if (clPos != std::string::npos) {
                clPos += 16;
                size_t clEnd = header.find("\r\n", clPos);
                std::string clStr = header.substr(clPos, clEnd - clPos);
                Utils::atoi(clStr.c_str(), (int&)eventstrct->contentLength);
            } else {
                size_t methodEnd = header.find(' ');
                std::string method = header.substr(0, methodEnd);
                if (method == "POST" || method == "PUT" || method == "PATCH")
                    throw HttpException(411, "Length Required");
            }
        }
    } else if (eventstrct->content.size() > LIMIT_HEADER_SIZE) {
        throw HttpException(431, "Header too large");
    }
}

bool	EventPool::processChunk(eventStructTmp* eventstrct, size_t &headerEnd)
{
	std::string body = eventstrct->content.substr(headerEnd + 4);
    while (true) {
        size_t pos = body.find("\r\n");
        if (pos == std::string::npos)
            break; // aún no tenemos tamaño de chunk
        std::string chunkSizeStr = body.substr(0, pos);
        int chunkSize;
        std::stringstream ss;
        ss << std::hex << chunkSizeStr;
        ss >> chunkSize;
        if (chunkSize == 0) {
            return true;
        }
        if (body.size() < pos + 2 + chunkSize + 2)
            return false; // falta parte del chunk
        std::string chunkData = body.substr(pos + 2, chunkSize);
        eventstrct->bodyDecoded.append(chunkData);
        body = body.substr(pos + 2 + chunkSize + 2);
        eventstrct->content = eventstrct->content.substr(0, headerEnd + 4) + body;
    }
	return false;
}

bool EventPool::getRequest(int socketFd, eventStructTmp* eventstrct) {
    char buffer[4096];
    ssize_t bytes_read = 0;
    bool readSome = false;
    bool done = false;

    while (42) {
        bytes_read = recv(socketFd, buffer, sizeof(buffer), 0);

        if (bytes_read > 0) {
            readSome = true;
            eventstrct->content.append(buffer, bytes_read);
            eventstrct->offset += bytes_read;
            if (eventstrct->headerParsed && !eventstrct->isChunked) {
              checkBodySize(eventstrct);
            }
            if (!eventstrct->headerParsed) {
				parseHeader(eventstrct);
            }
            if (eventstrct->headerParsed) { //No else ya que parseHeader puede cambiar el estado.
				size_t headerEnd = eventstrct->content.find("\r\n\r\n");
                if (eventstrct->isChunked) {
					done = processChunk(eventstrct, headerEnd);
                } else {
                    std::string body = eventstrct->content.substr(headerEnd + 4);
                    if (body.size() >= eventstrct->contentLength)
                        done = true;
                }
            }
        }
        else if (bytes_read == 0) {
            throw disconnectedException(socketFd);
        }
        else {
            if (readSome)
                break;
            else
                throw socketReadException(socketFd);
        }
    }
    return done;
}

void EventPool::handleClientRequest(int fd, eventStructTmp *eventStrct)
{
	try {

		if (!getRequest(fd, eventStrct))
			return;
		Logger::log(str("Received request: ") + eventStrct->content, INFO);
		HttpRequest request(eventStrct);
		HttpResponse response = stablishResponse(request, eventStrct->server, eventStrct);
    if (response._cgiNonBlock)
      return ;
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
  strMap::const_iterator it;
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
  result->headerParsed = false;
	result->isChunked = false;
	result->contentLength = 0;
	result->bodyDecoded = "";
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
    processEvents();
  }
}

int EventPool::setFd(eventStructTmp *eventStrct)
{
    switch (eventStrct->eventType)
    {
    case NEWCONNECTION:
      return  eventStrct->server->getServerFd();
      break;
    case RECIEVEREQUEST:
      return  eventStrct->client_fd;
      break;
    case SENDRESPONSE:
      return  eventStrct->client_fd;
      break;
    case CGISENDING:
      return  eventStrct->cgiData.pipeIn;
      break;
    case CGIREADING:
      return  eventStrct->cgiData.pipeOut;
      break;
    default:
    return 0;
      break;
    }
}

void EventPool::processEvents() {
  for (int i = 0; i < _nfds; ++i) {
    int fd;
    eventStructTmp *eventStrct =
        static_cast<eventStructTmp *>(events[i].data.ptr);
    fd = setFd(eventStrct);
    uint32_t flags = events[i].events;
    if (flags & (EPOLLHUP | EPOLLERR)) {
      Logger::log(str("Closing fd: ") + Utils::intToStr(fd) +
                      " Because EPOLLERR or EPOLLHUP",
                  ERROR);
      safeCloseAndDelete(fd, eventStrct);
      continue ;
    }
    switch (eventStrct->eventType)
    {
      case NEWCONNECTION:
        handleClientConnection(fd, eventStrct);
        break;
      case RECIEVEREQUEST:
        handleClientRequest(fd, eventStrct);
        break;
      case SENDRESPONSE:
        handleClientWrite(fd, eventStrct);
        break;
      case CGISENDING:
        handleCgiWrite(fd, eventStrct);
        break;
      case CGIREADING:
        handleCgiRead(fd, eventStrct);
        break;
    }
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
	delete eventStruct;
	eventStruct = NULL;
}

HttpResponse			EventPool::stablishResponse(HttpRequest &request, Server *server, eventStructTmp *eventStrct)
{
  if (request.getPayLoad())
    return Utils::codeResponse(502, server);
  if (request.getBadRequest())
    return Utils::codeResponse(400, server);
  else if (!request.getRedirect().empty())
    return HttpResponse(request, server);
  else if(!request.getCanAccess())
    return Utils::codeResponse(403, server);
  else if (!request.getLocation())
    return Utils::codeResponse(404, server);
  else if (!request.getIsCgi() && !request.getValidMethod())
    return Utils::codeResponse(405, server);
  else if (!request.getIsCgi() && (request.getReceivedMethod() == "GET" && !request.getResourceExists() && !request.getLocation()->getAutoindex()))
    return Utils::codeResponse(404, server);
  else if (request.getIsCgi() && !request.getResourceExists())
    return Utils::codeResponse(404, server);
	else if (request.getLocation()->getAutoindex() && request.getReceivedMethod() == "GET" && !request.getResourceExists()) {
    std::string uri("http://");
    uri.append(request.getHeader().find("Host")->second);
    uri.append(request.getUri());
    return HttpResponse(request, server, AutoIndex::getAutoIndex(request.getLocation()->getUrlPath(), uri, server->getRoot() + request.getLocation()->getRoot()));
  }
	else if (request.getIsCgi())
		return HttpResponse(request, eventStrct, _pollFd);
  else
    return HttpResponse(request, server);
}

#include <sys/wait.h>  // para waitpid

void EventPool::handleCgiRead(int fd, eventStructTmp* eventStrct) {
    char buf[8192];
    for (;;) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            eventStrct->cgiData.cgiRead.append(buf, n);
        }
        else if (n == 0) {
            // EOF: CGI terminó. Preparamos la respuesta HTTP
            close(fd);
            int status;
            waitpid(eventStrct->cgiData.cgiPid, &status, 0);

            HttpResponse resp;
            std::string body = resp.saveCgiHeader(eventStrct->cgiData.cgiRead);
            if (resp._line0.empty()) {
                resp._line0 = "HTTP/1.1 200 OK\r\n";
            }
            // montar headers + body en eventStrct->content
            std::string out = resp._line0;
            std::map<std::string,std::string> hdrs = resp.getHeader();
            for (std::map<std::string,std::string>::const_iterator it = hdrs.begin(); it != hdrs.end(); ++it) {
                out += it->first;
                out += ": ";
                out += it->second;
                out += "\r\n";
            }
            out += "\r\n";
            out += body;

            eventStrct->content   = out;
            eventStrct->offset    = 0;
            eventStrct->eventType = SENDRESPONSE;

            // pasar el client_fd a EPOLLOUT
            struct epoll_event ev;
            ev.events   = EPOLLOUT | EPOLLET;
            ev.data.ptr = static_cast<void*>(eventStrct);
            if (epoll_ctl(_pollFd, EPOLL_CTL_MOD, eventStrct->client_fd, &ev) == -1) {
                Logger::log("Failed to modify event to EPOLLOUT", ERROR);
            }
            return;
        }
        else if (errno == EAGAIN) {
            // sin más datos ahora, espera próximo EPOLLIN
            return;
        }
        else {
            // error: abortar CGI
            kill(eventStrct->cgiData.cgiPid, SIGKILL);
            safeCloseAndDelete(fd, eventStrct);
            return;
        }
    }
}

void EventPool::handleCgiWrite(int fd, eventStructTmp* eventStrct) {
    const char* ptr = eventStrct->cgiData.cgiWrite.data() + eventStrct->cgiData.writeOffset;
    size_t rem = eventStrct->cgiData.cgiWrite.size() - eventStrct->cgiData.writeOffset;

    while (rem > 0) {
        ssize_t n = write(fd, ptr, rem);
        if (n > 0) {
            eventStrct->cgiData.writeOffset += n;
            ptr  += n;
            rem  -= n;
        }
        else if (errno == EAGAIN) {
            // pipe lleno, espera próximo EPOLLOUT
            return;
        }
        else {
            // error grave: abortar CGI
            kill(eventStrct->cgiData.cgiPid, SIGKILL);
            safeCloseAndDelete(fd, eventStrct);
            return;
        }
    }

    // todo escrito: desuscribir EPOLLOUT y pasar a lectura
    if (epoll_ctl(_pollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        Logger::log("Failed to remove pipeIn from epoll", ERROR);
    }
    close(fd);
    eventStrct->eventType = CGIREADING;
}

int EventPool::getPollFd() {return _pollFd;}

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
