/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 18:47:18 by shurtado         ###   ########.fr       */
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

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server server;
// inicializamos socket
	int serverFd = socket(AF_INET, SOCK_STREAM, 0);
	int yes = 1;
// Poder reutilizar el mismo puerto sin tener que esperar en caso de fallo del programa
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	fcntl(serverFd, F_SETFL, O_NONBLOCK);
// carga el host name y port en el addrinfo
// hints -> asignamos opciones de config
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;			// IPv4
	hints.ai_socktype = SOCK_STREAM;	// TCP
	hints.ai_flags = AI_PASSIVE;		// Para bind() (escuchar)
// con las opciones dadas en hints configura res
	int err = getaddrinfo(server.getHostName().c_str(), server.getPort().c_str(), &hints, &res);
	if (err)
	{
		std::cout << "error: getaddrinfo" << std::endl << gai_strerror(err) << std::endl;
		return (1);
	}
// vincula fd del servidor al host name y port
	else if (bind(serverFd, res->ai_addr, res->ai_addrlen) == -1) {
		std::cout << "error: bind" << std::endl;
		return (1);
	}
// habilitamos el fd para que se quede escuchando
	listen(serverFd, SOMAXCONN);
// creamos el pool (epoll) de eventos
	int pollFd = epoll_create(1);
	if (pollFd == -1) {
		std::cout << "error: epoll_create1" << std::endl;
		return (1);
	}
// vincula fd del pool de eventos al host name y port
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = serverFd;
	if (epoll_ctl(pollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1) {
		std::cout << "error: epoll_create1" << std::endl;
		return 1;
	}
// esperamos a que se acceda al ej. localhost:8080
	struct epoll_event events[1024]; // tamaño = número máximo de eventos a esperar
	int nfds = epoll_wait(pollFd, events, 1024, -1); // -1 = bloquea indefinidamente
	if (nfds == -1) {
		std::cout << "error: epoll_wait" << std::endl;
		return 1;
	}
	close(serverFd);
	freeaddrinfo(res);
	return 0;
}
