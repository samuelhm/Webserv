/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/06 19:50:52 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/Server.hpp"
#include "EventPool/EventPool.hpp"
#include "ConfigFile/ParseConfig.hpp"

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
	Logger::log("Trying to get DebugLever form Env..", INFO);
	Logger::initFromEnv();
	if (ac != 2) {
		Logger::log(str("Usage: ") + av[0] + " Configfile", ERROR);
		return 0;
	}
	try {
		std::vector<Server*> Servers = parseConfigFile(av[1]);
		EventPool pool(Servers);
		pool.poolLoop(Servers);
		Utils::foreach(Servers.begin(), Servers.end(), Utils::deleteItem<Server>);
		Servers.clear();
	}
	catch (std::exception &e)
	{
		Logger::log(str("An Unmanaged exception caught on main!: ") + e.what(), ERROR);
	}
	return 0;
}
