/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/04 19:46:56 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../ConfigFile/Server.hpp"
#include "../HTTP/HttpRequest.hpp"
#include "../HTTP/HttpResponse.hpp"
#include "../Utils/AutoIndex.hpp"
#include "../Utils/Utils.hpp"
#include <cerrno>
#include <csignal>
#include <cstring>
#include <exception>
#include <poll.h>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

extern volatile sig_atomic_t epollRun;

class Server;
class HttpResponse;
class HttpRequest;

struct eventStructTmp {
  Server *server;
  int client_fd;
  EventType eventType;
  str content;
  size_t offset;

  // chunks
  bool headerParsed;
  bool isChunked;
  size_t contentLength;
  str bodyDecoded;
};

class EventPool {
private:
  int _pollFd;
  int _nfds;

  struct epoll_event events[1024];
  std::vector<struct eventStructTmp *> _structs;

  struct eventStructTmp *createEventStruct(int fd, Server *server,
                                           EventType eventType);
  void processEvents();
  void saveResponse(HttpResponse &response, eventStructTmp *eventStrct);
  bool getRequest(int socketFd, eventStructTmp *eventstrct);
  void handleClientRequest(int fd, eventStructTmp *eventStrct);
  void handleClientConnection(int fd, eventStructTmp *eventStrct);
  bool handleClientWrite(int fd, eventStructTmp *eventStrct);
  void safeCloseAndDelete(int fd, eventStructTmp *eventStruct);
  HttpResponse stablishResponse(HttpRequest &request, Server *server);
  bool headerTooLarge(str const &request, int &errorCode);
  bool setContentLength(eventStructTmp *eventstrct, int &content_lenght);
  void checkBodySize(eventStructTmp *eventstrct);
  void parseHeader(eventStructTmp *eventstrct);
  bool processChunk(eventStructTmp *eventstrct, size_t &headerEnd);

public:
  EventPool(std::vector<Server *> &Servers);
  ~EventPool();
  void poolLoop();
  void acceptConnection(int fdTmp, Server *server);

  class socketReadException : public std::exception {
  private:
    str message;

  public:
    socketReadException(int fd);
    const char *what() const throw();
    virtual ~socketReadException() throw() {}
  };
  class disconnectedException : public std::exception {
  private:
    str message;

  public:
    disconnectedException(int fd);
    const char *what() const throw();
    virtual ~disconnectedException() throw() {}
  };
  class AcceptConnectionException : public std::exception {
  private:
    str message;

  public:
    AcceptConnectionException(str const &msg);
    const char *what() const throw();
    virtual ~AcceptConnectionException() throw() {}
  };
  class HttpException : public std::exception {
  private:
    int _errorCode;
    str _message;

  public:
    HttpException(int errorCode, const str &extra = "");
    virtual const char *what() const throw();
    int getErrorCode() const throw();
    virtual ~HttpException() throw();
  };
};
