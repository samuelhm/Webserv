/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/08 14:01:06 by shurtado         ###   ########.fr       */
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
#include <csignal>

void signalQuit(int signum) {
	(void)signum;
	exit(0);
}
int main(int ac, char **av)
{
	if (signal(SIGQUIT, signalQuit) == SIG_ERR) {
		Logger::log("Error en SigQuit", ERROR);
		return 1;
	}
	Logger::log("Trying to get DebugLever form Env..", INFO);
	Logger::initFromEnv();
	if (ac != 2) {
		Logger::log(str("Usage: ") + av[0] + " Configfile", ERROR);
		return 1;
	}
	Utils::fillStatusStr();
	try {
		std::vector<Server*> Servers = parseConfigFile(av[1]);
		EventPool pool(Servers);
		pool.poolLoop(Servers);
		Utils::foreach(Servers.begin(), Servers.end(), Utils::deleteItem<Server>);
		Servers.clear();
	}
	catch (ConfigFileException &e) {
		Logger::log(e.what(), ERROR);
		return 1;
	}
	catch (std::exception &e)
	{
		Logger::log(str("An Unmanaged exception caught on main!: ") + e.what(), ERROR);
		Utils::foreach(Servers.begin(), Servers.end(), Utils::deleteItem<Server>);
		return 1;
	}
	return 0;
}
