/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/30 13:44:06 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/Server.hpp"
#include "EventPool/EventPool.hpp"

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

	EventPool pool(Servers);
	pool.poolLoop(Servers);
// creamos el pool (epoll) de eventos
// esperamos a que se acceda al ej. localhost:8080

	return 0;
}
