/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/26 16:49:47 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/Server.hpp"

#include <netdb.h> // getaddrinfo, addrinfo
#include <sys/socket.h> // socket, SOL_SOCKET, SO_REUSEADDR, setsockopt, AF_INET, SOCK_STREAM
#include <unistd.h> // close
#include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
#include <string.h> // memset
#include <iostream> // cout, endl
#include <sys/epoll.h> // epoll
#include <stdio.h>
#include <cerrno>
#include <cstdlib>

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	std::vector<Server*> Servers;
	Servers.push_back(new Server("servidor1", "8080"));
	Servers.push_back(new Server("servidor2", "8181"));


// creamos el pool (epoll) de eventos
	int pollFd = epoll_create(1);
	if (pollFd == -1) {
		std::cout << "error: epoll_create1" << std::endl;
		return (1);
	}

	struct epoll_event ev;

	for (std::vector<Server*>::iterator it = Servers.begin(); it != Servers.end(); ++it)
	{
		ev.events = EPOLLIN;
		ev.data.fd = (*it)->getServerFd();
		if (epoll_ctl(pollFd, EPOLL_CTL_ADD, (*it)->getServerFd(), &ev) == -1) {
			perror("epoll_ctl");
			return 1;
		}

	}

	int nfds;
// esperamos a que se acceda al ej. localhost:8080
	struct epoll_event events[1024]; // tamaño = número máximo de eventos a esperar
	while (1)
	{
		nfds = epoll_wait(pollFd, events, 1024, -1); // -1 = bloquea indefinidamente
		if (nfds == -1) {
			perror("epoll_wait");
			return 1;
		}
		for (int i = 0; i < nfds; ++i)
		{
			int fd = events[i].data.fd;

			uint32_t flags = events[i].events;
			if (flags & (EPOLLHUP | EPOLLERR)) {
				std::cout << "Cerrando conexión con fd: " << fd << std::endl;
				epoll_ctl(pollFd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				continue;
			}
			bool is_server_fd = false;
			for (size_t s = 0; s < Servers.size(); ++s) {
				if (fd == Servers[s]->getServerFd()) {
					is_server_fd = true;
					break;
				}
			}
			if (is_server_fd)
			{
				// 📌 ACEPTAR NUEVA CONEXIÓN
				struct sockaddr_in client_address;
				socklen_t client_len = sizeof(client_address);

				int client_fd = accept(fd, (struct sockaddr *)&client_address, &client_len);
				if (client_fd == -1) {
					perror("accept");
					continue;
				}

				std::cout << "Nueva conexión en fd: " << client_fd << std::endl;
				fcntl(client_fd, F_SETFL, O_NONBLOCK);

				struct epoll_event client_ev;
				client_ev.events = EPOLLIN;
				client_ev.data.fd = client_fd;

				epoll_ctl(pollFd, EPOLL_CTL_ADD, client_fd, &client_ev);
			}
			else
			{
				char buffer[4096];
				ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

				if (bytes_read <= 0) {
					if (bytes_read == 0)
					std::cout << "Cliente desconectado: fd = " << fd << std::endl;
					else
					perror("read");

					continue;
				}

				const char* http_response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 45\r\n"
				"\r\n"
				"<html><body><h1>Hola mundo</h1></body></html>";

				// Enviar la respuesta
				ssize_t error = write(fd, http_response, strlen(http_response));
				if (error == -1)
					perror("write");
				epoll_ctl(pollFd, EPOLL_CTL_DEL, fd, NULL);
				close(fd);
				buffer[bytes_read] = '\0';

				std::cout << "Petición HTTP recibida del fd (" << fd << "):\n";
				std::cout << buffer << std::endl;
				// 📌 Aquí podrías procesar la petición HTTP y enviar respuesta
			}
		}
	}
	return 0;
}
