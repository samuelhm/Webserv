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
    bool readSome = false;
    bool done = false;

    while (42) {
        bytes_read = recv(socketFd, buffer, sizeof(buffer), 0);

        if (bytes_read > 0) {
            readSome = true;
            eventstrct->content.append(buffer, bytes_read);
            eventstrct->offset += bytes_read;

            // Comprobación de Payload Too Large para Content-Length
            if (eventstrct->headerParsed && !eventstrct->isChunked) {
                size_t headerEnd = eventstrct->content.find("\r\n\r\n");
                std::string body = eventstrct->content.substr(headerEnd + 4);
                if (body.size() > eventstrct->server->getBodySize()) {
                    Logger::log("Payload too large (Content-Length body)", WARNING);
                    throw HttpException(413, "Payload Too Large");
                }
            }

            // Procesar header si no estaba parseado
            if (!eventstrct->headerParsed) {
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

            // Procesar body
            if (eventstrct->headerParsed) {
                size_t headerEnd = eventstrct->content.find("\r\n\r\n");

                if (eventstrct->isChunked) {
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
                            done = true;
                            break;
                        }

                        if (body.size() < pos + 2 + chunkSize + 2)
                            break; // falta parte del chunk

                        std::string chunkData = body.substr(pos + 2, chunkSize);
                        eventstrct->bodyDecoded.append(chunkData);
                        body = body.substr(pos + 2 + chunkSize + 2);
                        eventstrct->content = eventstrct->content.substr(0, headerEnd + 4) + body;
                    }
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
        else { // bytes_read < 0
            if (readSome)
                break; // hemos leído algo antes: salida normal
            else
                throw socketReadException(socketFd); // error real
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
