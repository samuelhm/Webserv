/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                    :+:      :+:    :+: */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 16:14:49 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../ConfigFile/Server.hpp"
#include "AHttp.hpp"
#include "HttpRequest.hpp"
#include <fstream>

class Server;
class HttpRequest;
struct epoll_event;

class HttpResponse : public AHttp {
public:
  str _line0;
  int _status;
  char **_envp;
  char **_argv;
  str _cgiOutput;
  bool _cgiSaveErr;
  void setErrorCode(int ErrorCode, Server *server);
  void cgiExec(const HttpRequest &request, Server *server);
  void cgiSaveItems(const HttpRequest &request);
  void cgiFree();
  void staticFileExec(const HttpRequest &request, Server *server);
  void staticFileGet(const HttpRequest &request, Server *server);
  void staticFilePost(const HttpRequest &request, Server *server);
  void staticFilePut(const HttpRequest &request, Server *server);
  void staticFileOptions(const HttpRequest &request, Server *server);
  void staticFileDelete(const HttpRequest &request, Server *server);
  void redirecResponse(const HttpRequest &request, Server *server);
  str saveCgiHeader(const str cgiOutput);
  void safeCloseCgiExec(Server *server, const str &msg);
  void childExec(int *pipeIn, int *pipeOut);
  void workerExec(int *pipeIn, int *pipeOut, Server *server,
                  const HttpRequest &request, pid_t &pid);
  void fillEvents(bool &pipeInOpen, epoll_event &evIn, epoll_event &evOut,
                  int epollFd, int *pipeIn, int *pipeOut);
  void readPipes(bool &pipeInOpen, size_t &totalWritten, size_t bodySize,
                 int epollFd, struct epoll_event &event, int *pipeIn,
                 const char *body);
  void writePipes(bool &pipeOutOpen, int epollFd, char *buffer,
                  struct epoll_event &event, int *pipeOut);

public:
  HttpResponse(const HttpRequest &request, Server *server);
  HttpResponse(int errorCode, Server *server);
  HttpResponse(const HttpRequest &request, Server *server, str autoIndexHTML);
  ~HttpResponse();
};
