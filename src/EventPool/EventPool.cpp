#include "EventPool.hpp"
#include "../Utils/AutoIndex.hpp"
#include "../Utils/Utils.hpp"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

EventPool::EventPool(std::vector<Server *> &Servers) {
  _pollFd = epoll_create(1);
  if (_pollFd == -1) {
    Logger::log("Error Creating epoll FD", ERROR);
    throw std::exception();
  }
  struct epoll_event ev;
  for (std::vector<Server *>::iterator it = Servers.begin();
       it != Servers.end(); ++it) {
    ev.events = EPOLLIN;
    struct eventStructTmp *serverStruct =
        createEventStruct((*it)->getServerFd(), *it, true);
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

bool EventPool::headerTooLarge(str const &request) {
	size_t end = request.find("\r\n\r\n");
	if (end == std::string::npos)
		return (request.size() > LIMIT_HEADER_SIZE);
	return (end > LIMIT_HEADER_SIZE);
}

str EventPool::getRequest(int socketFd) {
  char buffer[4096];
  std::memset(buffer, 0, 4096);
  size_t total_bytes = 0;
  ssize_t bytes_read = 0;

  struct pollfd pfd;
  pfd.fd = socketFd;
  pfd.events = POLLIN;

  poll(&pfd, 1, 0);
  str request;
  while (pfd.revents & POLLIN)
  {
    bytes_read = recv(socketFd, buffer, sizeof(buffer), MSG_DONTWAIT);
    if (bytes_read == -1)
      throw socketReadException(socketFd);
    total_bytes += bytes_read;
    request.append(buffer, bytes_read);
    std::memset(buffer, 0, bytes_read);
		if (request.size() >= LIMIT_HEADER_SIZE && headerTooLarge(request))
			throw headerTooLargeException(socketFd);
    poll(&pfd, 1, 0);
    if (pfd.revents & (POLLHUP | POLLERR))
      break;
  }

  if (total_bytes == 0)
    throw disconnectedException(socketFd);

  Logger::log(str("HTTP Request Received.") + request, INFO);
  return (request);
}

void EventPool::sendResponse(HttpResponse &response, int fdTmp,
                             const strMap &m) {
  str err;
  err.append(response._line0);
  strMap::iterator it;
  for (it = response.getHeader().begin(); it != response.getHeader().end();
       ++it) {
    err.append(it->first);
    err.append(": ");
    err.append(it->second);
    err.append("\r\n");
  }
  err.append(response.getBody());
  ssize_t error = write(fdTmp, err.c_str(), err.size());
  if (error == -1)
    Logger::log("Cannot Write on Socket!.", ERROR);
  Logger::log(str("Sending resource to fd: ") + Utils::intToStr(fdTmp), USER);
  Logger::log(str("Sending to client the header: \n") + Utils::returnMap(m),
              INFO);
  Logger::log(str("Sending to client the body:\n") + response.getBody(), INFO);
}

struct eventStructTmp *EventPool::createEventStruct(int fd, Server *server,
                                                    bool serverOrClient) {
  struct eventStructTmp *result = new eventStructTmp;
  result->client_fd = fd;
  result->server = server;
  result->isServer = serverOrClient;
  return result;
}

void EventPool::acceptConnection(int fdTmp, Server *server) {
  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);

  int client_fd =
      accept(fdTmp, (struct sockaddr *)&client_address, &client_len);
  if (client_fd == -1)
    throw AcceptConnectionException("Failed on Accept connetion for server: " +
                                    server->getServerName());
  if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
    close(client_fd);
    throw AcceptConnectionException(
        "Failed to set client with flag 0_NONBLOCK");
  }
  Logger::log(str("Accepted connection for server: ") +
                  server->getServerName() + " at port: " + server->getPort(),
              USER);
  struct epoll_event client_ev;
  client_ev.events = EPOLLIN;
  struct eventStructTmp *clientStruct =
      createEventStruct(client_fd, server, false);
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

void EventPool::poolLoop(std::vector<Server *> &Servers) {
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
    processEvents(Servers);
  }
}

void EventPool::processEvents(std::vector<Server *> &Servers) {
  for (int i = 0; i < _nfds; ++i) {
    int fd;
    eventStructTmp *eventStrct =
        static_cast<eventStructTmp *>(events[i].data.ptr);
    if (eventStrct->isServer)
      fd = eventStrct->server->getServerFd();
    else
      fd = eventStrct->client_fd;
    uint32_t flags = events[i].events;
    if (flags & (EPOLLHUP | EPOLLERR)) {
      Logger::log(str("Closing fd: ") + Utils::intToStr(fd) +
                      " Because EPOLLERR or EPOLLHUP",
                  ERROR);
      safeCloseAndDelete(fd, eventStrct);
      continue;
    }
    if (isServerFd(Servers, fd)) {
      handleClientConnection(fd, eventStrct);
      continue;
    } else
      handleClientRequest(fd, eventStrct);
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

bool EventPool::checkCGI(str path, Server server) {
  // if (!isCgi(path))
  // return false;
  str Path = path;
  str ext = path.substr(path.find_last_of('.'));
  Path.append("/");
  Path = AutoIndex::getPrevPath(Path);
  Path.erase(Path.size() - 1);
  std::vector<Location *> locations = server.getLocations();
  for (std::size_t i = 0; i < locations.size(); i++) {
    if (Path == locations[i]->getRoot()) {
      if (!locations[i]->getCgiEnable())
        return false;
      strVec extensions = locations[i]->getCgiExtension();
      for (std::size_t j = 0; j < extensions.size(); j++) {
        if (ext == extensions[j])
          return true;
      }
      return false;
    }
  }
  return false;
}

void EventPool::handleClientRequest(int fd, eventStructTmp *eventStrct) {
  try {
    str reqStr = getRequest(fd);
    HttpRequest request(reqStr, eventStrct->server);
    Utils::printRequest(request);
    HttpResponse response(request, eventStrct->server);
    sendResponse(response, fd, response.getHeader());
  } catch (const disconnectedException &e) {
    Logger::log(str("Disconnection occur: ") + e.what(), WARNING);
  } catch (const socketReadException &e) {
    Logger::log(str("Socket Read Error: ") + e.what(), WARNING);
  } catch (const headerTooLargeException &e) {
		HttpResponse  response(Utils::codeResponse(431));
    sendResponse(response, fd, response.getHeader());
    Logger::log(e.what(), WARNING);
  } catch (...) {
    Logger::log("UNKNOWN FATAL ERROR ON handleClientRequest", ERROR);
  }
  safeCloseAndDelete(fd, eventStrct);
}

void EventPool::safeCloseAndDelete(int fd, eventStructTmp *eventStruct) {
  if (epoll_ctl(_pollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
    Logger::log("EPOLL_CTL_DEL Failed", ERROR);
  if (!eventStruct->isServer) {
    if (close(fd) == -1)
      Logger::log(str("Failed closing FD: ") + Utils::intToStr(fd), ERROR);
  }
  delete eventStruct; // Jamas deberia ser nulo llegado a este punto.
  eventStruct = NULL; // Protección para errores en destructor.
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
}

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
