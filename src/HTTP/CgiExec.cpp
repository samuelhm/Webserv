#include "../Utils/Utils.hpp"
#include "HttpResponse.hpp"
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void HttpResponse::cgiFree() {
  for (char **p = _envp; *p != NULL; ++p)
    delete[] *p;
  delete[] _envp;
  for (char **p = _argv; *p != NULL; ++p)
    delete[] *p;
  delete[] _argv;
}

void HttpResponse::cgiSaveItems(const HttpRequest &request) {
  strVec env_vec;
  const strMap &header = request.getHeader();
  env_vec.push_back("REQUEST_METHOD=" + request.getReceivedMethod());
  env_vec.push_back("SERVER_PROTOCOL=HTTP/1.1");
  env_vec.push_back("GATEWAY_INTERFACE=CGI/1.1");
  env_vec.push_back("QUERY_STRING=" + request.getQueryString());
  env_vec.push_back("REDIRECT_STATUS=200");
  env_vec.push_back("PATH_INFO=" + request.getPathInfo());
  env_vec.push_back("SCRIPT_NAME=" + request.getResource());
  env_vec.push_back("SCRIPT_FILENAME=" + request.getLocalPathResource());
  env_vec.push_back("CONTENT_LENGTH=" +
                    Utils::intToStr(request.getBody().length()));
  if (header.count("Content-Type"))
    env_vec.push_back("CONTENT_TYPE=" + header.at("Content-Type"));
  env_vec.push_back("HTTP_COOKIE=" + request.getSessionUser());
  for (strMap::const_iterator it = header.begin(); it != header.end(); ++it) {
    std::string key = it->first;
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    std::replace(key.begin(), key.end(), '-', '_');
    env_vec.push_back("HTTP_" + key + "=" + it->second);
  }
  char **envp = new char *[env_vec.size() + 1];
  for (size_t i = 0; i < env_vec.size(); i++)
    envp[i] = strdup(env_vec[i].c_str());
  envp[env_vec.size()] = NULL;
  char **argv = new char *[3];
  argv[0] = strdup(request.getLocation()->getCgiPath().c_str());
  argv[1] = strdup(request.getLocalPathResource().c_str());
  argv[2] = NULL;
  _envp = envp;
  _argv = argv;
}

str HttpResponse::saveCgiHeader(const str cgiOutput) {
  size_t pos = 0;
  size_t end;
  bool headersDone = false;
  while (!headersDone && (end = cgiOutput.find("\r\n", pos)) != str::npos) {
    str line = cgiOutput.substr(pos, end - pos);
    if (line.empty()) {
      headersDone = true;
      pos = end + 2;
      break;
    }
    size_t separator = line.find(": ");
    if (separator != str::npos) {
      str key = line.substr(0, separator);
      str value = line.substr(separator + 2);
      if (key == "Status")
        _line0 = "HTTP/1.1 " + value + "\r\n";
      else
        _header[key] = value;
    }
    pos = end + 2;
  }
  return cgiOutput.substr(pos);
}

void HttpResponse::safeCloseCgiExec(Server *server, const str &msg) {
  Logger::log(msg, WARNING);
  cgiFree();
  setErrorCode(500, server);
}

void HttpResponse::childExec(int *pipeIn, int *pipeOut) {
  close(pipeIn[1]);
  dup2(pipeIn[0], STDIN_FILENO);
  close(pipeIn[0]);
  close(pipeOut[0]);
  dup2(pipeOut[1], STDOUT_FILENO);
  close(pipeOut[1]);

  execve(_argv[0], _argv, _envp);
  perror("execve");
  exit(1);
}

void HttpResponse::fillEvents(bool &pipeInOpen, epoll_event &evIn,
                              epoll_event &evOut, int epollFd, int *pipeIn,
                              int *pipeOut) {
  if (pipeInOpen) {
    evIn.events = EPOLLOUT;
    evIn.data.fd = pipeIn[1];
    epoll_ctl(epollFd, EPOLL_CTL_ADD, pipeIn[1], &evIn);
  } else {
    close(pipeIn[1]);
  }
  evOut.events = EPOLLIN;
  evOut.data.fd = pipeOut[0];
  epoll_ctl(epollFd, EPOLL_CTL_ADD, pipeOut[0], &evOut);
}

void HttpResponse::writePipes(bool &pipeOutOpen, int epollFd, char *buffer,
                              struct epoll_event &event, int *pipeOut) {
  if (pipeOutOpen && event.data.fd == pipeOut[0]) {
    ssize_t bytes = read(pipeOut[0], buffer, sizeof(buffer));
    if (bytes > 0) {
      _cgiOutput.append(buffer, bytes);
    } else if (bytes == 0) {
      epoll_ctl(epollFd, EPOLL_CTL_DEL, pipeOut[0], NULL);
      close(pipeOut[0]);
      pipeOutOpen = false;
    }
  }
}

void HttpResponse::readPipes(bool &pipeInOpen, size_t &totalWritten,
                             size_t bodySize, int epollFd,
                             struct epoll_event &event, int *pipeIn,
                             const char *body) {
  if (pipeInOpen && event.data.fd == pipeIn[1]) {
    if (totalWritten < bodySize) {
      ssize_t written =
          write(pipeIn[1], body + totalWritten, bodySize - totalWritten);
      if (written > 0) {
        totalWritten += written;
        if (totalWritten == bodySize) {
          epoll_ctl(epollFd, EPOLL_CTL_DEL, pipeIn[1], NULL);
          close(pipeIn[1]);
          pipeInOpen = false;
        }
      }
    }
  }
}

void HttpResponse::workerExec(int *pipeIn, int *pipeOut, Server *server,
                              const HttpRequest &request, pid_t &pid) {
  close(pipeIn[0]);
  close(pipeOut[1]);

  int epollFd = epoll_create1(0);
  if (epollFd == -1)
    return safeCloseCgiExec(server, "Failed to create epoll");

  struct epoll_event ev_in, ev_out;
  size_t bodySize = request.getBody().size();
  size_t totalWritten = 0;

  bool pipeInOpen = (bodySize > 0);
  bool pipeOutOpen = true;
  fillEvents(pipeInOpen, ev_in, ev_out, epollFd, pipeIn, pipeOut);
  char buffer[8192];

  while (pipeInOpen || pipeOutOpen) {
    struct epoll_event events[2];
    int n = epoll_wait(epollFd, events, 2, 5000); // timeout de 5 segundos
    if (n <= 0) {
      kill(pid, SIGKILL);
      if (pipeInOpen)
        close(pipeIn[1]);
      if (pipeOutOpen)
        close(pipeOut[0]);
      waitpid(pid, NULL, 0);
      return safeCloseCgiExec(server, n == 0 ? "Timeout in CGI execution"
                                             : "epoll_wait error");
    }

    for (int i = 0; i < n; ++i) {
      readPipes(pipeInOpen, totalWritten, bodySize, epollFd, events[i], pipeIn,
                request.getBody().c_str());
      writePipes(pipeOutOpen, epollFd, buffer, events[i], pipeOut);
    }
  }

  close(epollFd);

  int status;
  waitpid(pid, &status, 0);
  cgiFree();

  if ((WIFEXITED(status) && WEXITSTATUS(status)) || _cgiOutput.empty())
    return setErrorCode(500, server);

  _body = saveCgiHeader(_cgiOutput);
  if (_line0.empty())
    _line0 = "HTTP/1.1 200 OK\r\n";
  if (_cgiSaveErr)
    setErrorCode(500, server);
}

void HttpResponse::cgiExec(const HttpRequest &request, Server *server) {
  cgiSaveItems(request);
  int pipeIn[2];
  int pipeOut[2];

  if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
    return safeCloseCgiExec(server, "Failed to create pipes");

  fcntl(pipeIn[1], F_SETFL, O_NONBLOCK);
  fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);

  pid_t pid = fork();
  if (pid < 0)
    return safeCloseCgiExec(server, "Failed to fork");
  else if (pid == 0) {
    childExec(pipeIn, pipeOut);
  } else {
    workerExec(pipeIn, pipeOut, server, request, pid);
  }
}
