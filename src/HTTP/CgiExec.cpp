/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/25 19:16:54 by fcarranz         ###   ########.fr       */
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

#include <iostream>
#include <unistd.h>     // for getpid()
#include <sstream>      // for stringstream
#include <string>       // for std::string

str	HttpResponse::addUser(strMap &header) {
	static strVec	_users;
	bool			createUser = false;
	str				cookie = header["Cookie"];
	if (cookie.empty())
		createUser = true;
	for (size_t i = 0; i < _users.size(); i++) {
		if (_users[i] == cookie)
			return cookie;
	}
	if (!createUser) {
		_users.push_back(cookie);
		return cookie;
	}
	else {
		pid_t pid = getpid();
		std::stringstream ss;
		ss << pid;
		std::string SessionID = ss.str(); 
		_users.push_back(SessionID);
		return SessionID;
	}
}

void HttpResponse::cgiSaveItems(const HttpRequest &request) {
	strVec env_vec;
	env_vec.push_back("REQUEST_METHOD=" + request.getReceivedMethod());
	strMap	header = request.getHeader();
	
	if (!request.getQueryString().empty())
		env_vec.push_back("QUERY_STRING=" + request.getQueryString());
	if (!request.getPathInfo().empty())
		env_vec.push_back("PATH_INFO=" + request.getPathInfo() + "/" + request.getResource());
	if (!header["Cookie"].empty())
	env_vec.push_back("HTTP_COOKIE=" + addUser(header));	// IMPORTANT ADD THIS SOMEWHERE ELSE
	char** envp = new char*[env_vec.size() + 1];
	for (size_t i = 0; i < env_vec.size(); i++) {
		envp[i] = strdup(env_vec[i].c_str());
		std::cout << envp[i] << std::endl;
	}
	envp[env_vec.size()] = NULL;

	char** argv = new char*[2];
	argv[0] = strdup(request.getLocalPathResource().c_str());
	argv[1] = NULL;
	_envp = envp;
	_argv = argv;
}

str	HttpResponse::saveCgiHeader(const str cgiOutput) {
	size_t end = cgiOutput.find("\r\n");

    if (end == str::npos) {
		_cgiSaveErr = true;
		return "";
	}
    str line = cgiOutput.substr(0, end);
	if (line.empty())
        return cgiOutput.substr(end + 2);
    size_t separator = line.find(": ");
    if (separator != str::npos) {
        str key = line.substr(0, separator);
        str value = line.substr(separator + 2);
        _header[key] = value;
    }
	if (end + 2 < cgiOutput.length())
		return saveCgiHeader(cgiOutput.substr(end + 2));
	return cgiOutput;
}

void HttpResponse::replaceNewlines() {
	size_t		pos = 0;
	while ((pos = _cgiOutput.find('\n', pos)) != str::npos) {
		_cgiOutput.replace(pos, 1, "\r\n");
		pos += 2;
	}
}

void HttpResponse::cgiExec(const HttpRequest &request, Server *server) {
	cgiSaveItems(request);

	int pipe_fd[2];
	if (pipe(pipe_fd) == -1) {
		Logger::log("Failed to create pipe", ERROR);
		cgiFree();
		setErrorCode(500, server);
		return;
	}
	
	fd_set	readFds;
	FD_ZERO(&readFds);
	FD_SET(pipe_fd[0], &readFds);

	timeval time_out;
	time_out.tv_sec = TIMEOUT_CGI;
	time_out.tv_usec = 0;

	pid_t pid = fork();
	if (pid < 0) {
		Logger::log("Failed to fork", ERROR);
		cgiFree();
		setErrorCode(500, server);
		return ;
	}
	else if (pid == 0) {
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);

		if (execve(_argv[0], _argv, _envp) == -1) {
			perror("execve");
			exit(1);
		}
	}
	else {
		close(pipe_fd[1]);

		int control = select(pipe_fd[0] + 1, &readFds, NULL, NULL, &time_out);
		if (control == 0) {
			Logger::log("CGI timeout. Kill process", ERROR);
			kill(pid, SIGQUIT);
		}
		if (control > 0) {

			char buffer[1024];
			ssize_t bytes;

			while ((bytes = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
				_cgiOutput.append(buffer, bytes);
		}
		// if (control == -1) { } // Que hacemos si falla select()?

		close(pipe_fd[0]);
		int status;
		waitpid(pid, &status, 0);
		cgiFree();
		if ((WIFEXITED(status) && WEXITSTATUS(status)) || _cgiOutput.empty())
			return setErrorCode(500, server);
		replaceNewlines();
		_body = saveCgiHeader(_cgiOutput);
		std::cout << _cgiOutput << std::endl;
		_line0 = "HTTP/1.1 200 OK/r/n";
		if (_cgiSaveErr)
			setErrorCode(500, server);
	}
}
