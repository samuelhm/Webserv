/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/26 13:37:25 by shurtado         ###   ########.fr       */
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

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	std::vector<Server*> Servers;
	Servers.push_back(new Server("servidor1", "8080"));
	// Servers.push_back(new Server("servidor2", "8181"));


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
			std::cout << "error: epoll_wait" << std::endl;
			return 1;
		}
		// std::cout << " Server FD: "<< serve << "\nPollFD: "<< pollFd << "\nnfds: "<< nfds << "\nstdout: "<< STDOUT_FILENO << std::endl;
	}


	return 0;
}
