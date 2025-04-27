/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiExec.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 12:44:41 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/27 01:29:21 by shurtado         ###   ########.fr       */
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
            // 🔥 Línea vacía: fin de headers
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

        pos = end + 2; // Mover al siguiente \r\n
    }

    // 🔥 Lo que queda es el body real
    return cgiOutput.substr(pos);
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

	int pipe_in[2];  // Para mandar el body al hijo
	int pipe_out[2]; // Para leer la salida del hijo

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
		Logger::log("Failed to create pipes", ERROR);
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
		return;
	}
	else if (pid == 0) {
		// Código hijo
		close(pipe_in[1]);  // Cerramos lado de escritura del pipe_in
		dup2(pipe_in[0], STDIN_FILENO); // Redirigimos STDIN
		close(pipe_in[0]);

		close(pipe_out[0]); // Cerramos lado de lectura del pipe_out
		dup2(pipe_out[1], STDOUT_FILENO); // Redirigimos STDOUT
		close(pipe_out[1]);

		execve(_argv[0], _argv, _envp);

		// Si execve falla
		perror("execve");
		exit(1);
	}
	else {
		// Código padre
		close(pipe_in[0]);  // No leeremos de pipe_in
		close(pipe_out[1]); // No escribiremos en pipe_out

		//Si tenemos body que enviar, escribirlo al CGI
		if (!request.getBody().empty()) {
		    const char* body_ptr = request.getBody().c_str();
		    size_t body_size = request.getBody().size();
		    size_t total_written = 0;

		    Logger::log("Empiezo a escribir el body al CGI", WARNING);
			Logger::log(request.getBody(), INFO);

		    while (total_written < body_size) {
		        ssize_t written = write(pipe_in[1], body_ptr + total_written, std::min<size_t>(8192, body_size - total_written));
		        if (written == -1) {
		            Logger::log("Error escribiendo en el pipe hacia el CGI", ERROR);
		            break;
		        }
		        total_written += written;
		    }

		    Logger::log("Body escrito completamente al CGI", WARNING);
		}

		close(pipe_in[1]);

		//leer la salida del CGI
		char buffer[8192];
		ssize_t bytes;
		while (true) {
		    bytes = read(pipe_out[0], buffer, sizeof(buffer));
		    if (bytes > 0)
		        _cgiOutput.append(buffer, bytes);
		    else if (bytes == 0)
		        break; //EOF -> El hijo cerró stdout
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
