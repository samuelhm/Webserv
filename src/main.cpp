/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/02 22:11:03 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/ParseConfig.hpp"
#include "ConfigFile/Server.hpp"
#include "EventPool/EventPool.hpp"
#include "Utils/Logger.hpp"

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

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
  signal(SIGPIPE, SIG_IGN);
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
      pool.poolLoop();
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
