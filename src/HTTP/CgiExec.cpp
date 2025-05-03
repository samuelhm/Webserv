/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/05/03 00:09:52 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
  env_vec.push_back("PATH_INFO=" + request.getPathInfo());
  env_vec.push_back("SCRIPT_NAME=" + request.getResource());
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
  Logger::log(msg, ERROR);
  cgiFree();
  setErrorCode(500, server);
}

void HttpResponse::cgiExec(const HttpRequest &request, Server *server) {
  cgiSaveItems(request);
  int pipe_in[2];
  int pipe_out[2];

  if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    return safeCloseCgiExec(server, "Failed to create pipes");

  fcntl(pipe_in[1], F_SETFL, O_NONBLOCK);
  fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

  pid_t pid = fork();
  if (pid < 0)
    return safeCloseCgiExec(server, "Failed to fork");
  else if (pid == 0) {
    close(pipe_in[1]);
    dup2(pipe_in[0], STDIN_FILENO);
    close(pipe_in[0]);
    close(pipe_out[0]);
    dup2(pipe_out[1], STDOUT_FILENO);
    close(pipe_out[1]);

    execve(_argv[0], _argv, _envp);
    perror("execve");
    exit(1);
  } else {
    close(pipe_in[0]);
    close(pipe_out[1]);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
      return safeCloseCgiExec(server, "Failed to create epoll");

    struct epoll_event ev_in, ev_out;
    const char *body_ptr = request.getBody().c_str();
    size_t body_size = request.getBody().size();
    size_t total_written = 0;

    bool pipe_out_open = true;
    bool pipe_in_open = (body_size > 0);

    if (pipe_in_open) {
      ev_in.events = EPOLLOUT | EPOLLET;
      ev_in.data.fd = pipe_in[1];
      epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_in[1], &ev_in);
    } else {
      close(pipe_in[1]);
    }

    ev_out.events = EPOLLIN | EPOLLET;
    ev_out.data.fd = pipe_out[0];
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_out[0], &ev_out);

    char buffer[8192];

    while (pipe_in_open || pipe_out_open) {
      struct epoll_event events[2];
      int n = epoll_wait(epoll_fd, events, 2, 5000); // 5 sec timeout
      if (n == -1) {
        kill(pid, SIGKILL);
        if (pipe_in_open)
          close(pipe_in[1]);
        if (pipe_out_open)
          close(pipe_out[0]);
        waitpid(pid, NULL, 0);
        return safeCloseCgiExec(server, "epoll_wait error");
      }
      if (n == 0) {
        kill(pid, SIGKILL);
        if (pipe_in_open)
          close(pipe_in[1]);
        if (pipe_out_open)
          close(pipe_out[0]);
        waitpid(pid, NULL, 0);
        return safeCloseCgiExec(server, "Timeout in CGI execution");
      }
      for (int i = 0; i < n; ++i) {
        if (pipe_in_open && events[i].data.fd == pipe_in[1]) {
          while (total_written < body_size) {
            ssize_t written = write(pipe_in[1], body_ptr + total_written,
                                    body_size - total_written);
            if (written == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
              if (errno == EPIPE) {
                Logger::log("CGI cerró stdin prematuramente (EPIPE)", WARNING);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_in[1], NULL);
                close(pipe_in[1]);
                pipe_in_open = false;
                break;
              }
              kill(pid, SIGKILL);
              close(pipe_in[1]);
              close(pipe_out[0]);
              waitpid(pid, NULL, 0);
              return safeCloseCgiExec(server, "Write to pipe_in failed");
            }
            total_written += written;
          }
          if (total_written == body_size) {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_in[1], NULL);
            close(pipe_in[1]);
            pipe_in_open = false;
          }
        }
        if (pipe_out_open && events[i].data.fd == pipe_out[0]) {
          while (true) {
            ssize_t bytes = read(pipe_out[0], buffer, sizeof(buffer));
            if (bytes > 0)
              _cgiOutput.append(buffer, bytes);
            else if (bytes == 0) {
              epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_out[0], NULL);
              close(pipe_out[0]);
              pipe_out_open = false;
              break;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK)
              break;
            else {
              epoll_ctl(epoll_fd, EPOLL_CTL_DEL, pipe_out[0], NULL);
              close(pipe_out[0]);
              pipe_out_open = false;
              Logger::log("Error reading CGI output", WARNING);
              break;
            }
          }
        }
      }
    }
    close(epoll_fd);

    int status;
    waitpid(pid, &status, 0);
    cgiFree();

    Logger::log("CGI raw output: \n" + _cgiOutput, INFO);

    if ((WIFEXITED(status) && WEXITSTATUS(status)) || _cgiOutput.empty())
      return setErrorCode(500, server);

    _body = saveCgiHeader(_cgiOutput);
    if (_line0.empty())
      _line0 = "HTTP/1.1 200 OK\r\n";
    if (_cgiSaveErr)
      setErrorCode(500, server);
  }
}
