/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/19 13:02:40 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/ParseConfig.hpp"
#include "ConfigFile/Server.hpp"
#include "EventPool/EventPool.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>  // fcntl, F_SETFL, O_NONBLOCK
#include <iostream> // cout, endl
#include <netdb.h>  // getaddrinfo, addrinfo
#include <stdio.h>
#include <string.h>    // memset
#include <sys/epoll.h> // epoll
#include <sys/socket.h> // socket, SOL_SOCKET, SO_REUSEADDR, setsockopt, AF_INET, SOCK_STREAM
#include <sys/wait.h>
#include <unistd.h> // close

volatile sig_atomic_t epollRun = 1;

void signalQuit(int signum) {
  (void)signum;
  epollRun = 0;
}

void initWorkes(std::vector<int> &pids, int &active_workers) {
  for (; active_workers < NUMBER_OF_WORKERS; active_workers++) {
    pids[active_workers] = fork();
    if (pids[active_workers] == 0 || pids[active_workers] == -1)
      break;
  }
  if (active_workers == NUMBER_OF_WORKERS)
    --active_workers;
}

int main(int ac, char **av) {
  if (signal(SIGQUIT, signalQuit) == SIG_ERR ||
      signal(SIGINT, signalQuit) == SIG_ERR) {
    Logger::log("Error en señal", ERROR);
    return (1);
  }
  Logger::log("Trying to get DebugLever form Env..", INFO);
  Logger::initFromEnv();
  if (ac != 2) {
    Logger::log(str("Usage: ") + av[0] + " Configfile", ERROR);
    return 1;
  }
  Utils::fillStatusStr();
  try {
    std::vector<Server *> Servers = parseConfigFile(av[1]);

    std::vector<int> pids(NUMBER_OF_WORKERS);
    int active_workers = 0;
    if (NUMBER_OF_WORKERS > 0)
      initWorkes(pids, active_workers);

    if (pids.size() == 0 || pids[active_workers] == 0) {
      if (!Utils::setUpServers(Servers)) {
        Utils::foreach (Servers.begin(), Servers.end(),
                        Utils::deleteItem<Server>);
        Logger::log("Fail setting up servers", ERROR);
        return 1;
      }
      EventPool pool(Servers);
      pool.poolLoop(Servers);
      Utils::foreach (Servers.begin(), Servers.end(),
                      Utils::deleteItem<Server>);
      Servers.clear();
    } else {
      for (int i = 0; i < active_workers; i++) {
        waitpid(pids[i], NULL, 0);
      }
    }
  } catch (ConfigFileException &e) {
    Logger::log(e.what(), ERROR);
    if (!e.getServer().empty())
      Utils::foreach (e.getServer().begin(), e.getServer().end(),
                      Utils::deleteItem<Server>);
    return 1;
  } catch (std::exception &e) {
    Logger::log(e.what(), ERROR);
    return 1;
  }
  return 0;
}
