/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/05/02 20:19:48 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "../Utils/Utils.hpp"
#include <csignal>
#include <cstring>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>

void HttpResponse::cgiFree() {
	for (char** p = _envp; *p != NULL; ++p)
		delete[] *p;
	delete[] _envp;

	for (char** p = _argv; *p != NULL; ++p)
		delete[] *p;
	delete[] _argv;
}

void HttpResponse::cgiSaveItems(const HttpRequest &request) {
	strVec env_vec;
	env_vec.push_back("REQUEST_METHOD=" + request.getReceivedMethod());
	strMap	header = request.getHeader();

	if (!request.getQueryString().empty())
		env_vec.push_back("QUERY_STRING=" + request.getQueryString());
	if (!request.getPathInfo().empty())
		env_vec.push_back("PATH_INFO=" + request.getPathInfo());
	// if (!header["Cookie"].empty())
	env_vec.push_back("HTTP_COOKIE=" + request.getSessionUser());
	char** envp = new char*[env_vec.size() + 1];
	for (size_t i = 0; i < env_vec.size(); i++)
		envp[i] = strdup(env_vec[i].c_str());
	envp[env_vec.size()] = NULL;

	char** argv = new char*[3];
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

void	 HttpResponse::safeCloseCgiExec(Server *server, const str &msg) {
	Logger::log(msg, ERROR);
	cgiFree();
	_cgiNonBlock = false;
	setErrorCode(500, server);
}

void HttpResponse::cgiExec(const HttpRequest &request, eventStructTmp *eventStrct, int pollFd) {
	cgiSaveItems(request);
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) < 0 || pipe(pipeOut) < 0)
		return safeCloseCgiExec(eventStrct->server, "Failed to create pipes");

	fcntl(pipeIn[1],  F_SETFL, O_NONBLOCK);
  	fcntl(pipeIn[1],  F_SETFD, FD_CLOEXEC);
  	fcntl(pipeOut[0], F_SETFL, O_NONBLOCK);
  	fcntl(pipeOut[0], F_SETFD, FD_CLOEXEC);

	pid_t pid = fork();
	if (pid < 0)
		return safeCloseCgiExec(eventStrct->server, "Failed to fork"); //Important, no estamos cerrando pipes aqui.
	else if (pid == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[0]);
		close(pipeIn[1]);
		close(pipeOut[0]);
		close(pipeOut[1]);

		execve(_argv[0], _argv, _envp);
		perror("execve");
		exit(1);
	}
	else {
		close(pipeIn[0]);
		close(pipeOut[1]);

		eventStrct->cgiData.pipeIn = pipeIn[1];
		eventStrct->cgiData.pipeOut       = pipeOut[0];
  		eventStrct->cgiData.cgiPid        = pid;
  		eventStrct->cgiData.cgiWrite      = request.getBody();
  		eventStrct->cgiData.writeOffset   = 0;
  		eventStrct->cgiData.cgiRead.clear();
  		eventStrct->cgiData.HeadersParsed = false;

		struct epoll_event ev;
  		ev.data.ptr = eventStrct;
  		ev.events   = EPOLLOUT | EPOLLET;
  		epoll_ctl(pollFd, EPOLL_CTL_ADD, pipeIn[1],  &ev);
  		ev.events   = EPOLLIN  | EPOLLET;
  		epoll_ctl(pollFd, EPOLL_CTL_ADD, pipeOut[0], &ev);
		eventStrct->eventType = CGISENDING;
	}
}
