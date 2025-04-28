/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/28 11:25:46 by shurtado         ###   ########.fr       */
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
	setErrorCode(500, server);
}

void HttpResponse::cgiExec(const HttpRequest &request, Server *server) {
	cgiSaveItems(request);
	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		return safeCloseCgiExec(server, "Failed to create pipes");

	fd_set	readFds;
	FD_ZERO(&readFds);
	FD_SET(pipe_fd[0], &readFds);

	timeval time_out;
	time_out.tv_sec = TIMEOUT_CGI;
	time_out.tv_usec = 0;

	pid_t pid = fork();
	if (pid < 0)
		return safeCloseCgiExec(server, "Failed to fork"); //Important, no estamos cerrando pipes aqui.
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
	}
	else {
		close(pipe_in[0]);
		close(pipe_out[1]);
		if (!request.getBody().empty()) {
		    const char* body_ptr = request.getBody().c_str();
		    size_t body_size = request.getBody().size();
		    size_t total_written = 0;
		    while (total_written < body_size) {
		        ssize_t written = write(pipe_in[1], body_ptr + total_written, std::min<size_t>(8192, body_size - total_written));
		        if (written == -1) {
		            kill(pid, SIGKILL);
					waitpid(pid, NULL, 0);
					close(pipe_out[0]);
					return safeCloseCgiExec(server, "Fail to write pipe.");
		        }
		        total_written += written;
		    }
		}
		close(pipe_in[1]);
		char buffer[8192];
		ssize_t bytes;
		while (true) {
		    bytes = read(pipe_out[0], buffer, sizeof(buffer));
		    if (bytes > 0)
		        _cgiOutput.append(buffer, bytes);
		    else if (bytes == 0)
		        break;
		    else {
		        Logger::log("Error reading CGI output", WARNING);
		        break;
		    }
		}
		close(pipe_out[0]);
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
}
