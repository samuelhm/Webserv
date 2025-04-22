/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/18 13:04:31 by erigonza         ###   ########.fr       */
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
	str			method = "REQUEST_METHOD=" + request.getReceivedMethod();
	str			query = "QUERY_STRING=" + request.getQueryString();
	str			path   = "PATH_INFO=" + request.getPathInfo() + "/" + request.getResource();

	char** envp = new char*[4];
	envp[0] = strdup(method.c_str());
	envp[1] = query.empty() ? NULL : strdup(query.c_str());
	envp[2] = path.empty() ? NULL : strdup(path.c_str());
	envp[3] = NULL;

	char** argv = new char*[2];
	argv[0] = strdup(request.getLocalPathResource().c_str());
	argv[1] = NULL;
	_envp = envp;
	_argv = argv;
}

str	HttpResponse::saveCgiHeader(const str	cgiOutput) {
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
		_line0 = "HTTP/1.1 200 OK/r/n";
		if (_cgiSaveErr)
			setErrorCode(500, server);
	}
}
