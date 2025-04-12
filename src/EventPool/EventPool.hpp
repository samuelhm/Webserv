/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/12 13:19:59 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include <csignal>
#include <sys/epoll.h>
#include <exception>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stack>
#include "../ConfigFile/Server.hpp"
#include "../HTTP/HttpRequest.hpp"
#include "../HTTP/HttpResponse.hpp"

extern volatile sig_atomic_t epollRun;

struct eventStructTmp
{
	Server *server;
	int		client_fd;
	bool	isServer;
};

class EventPool {
	private:
		int		_pollFd;
		int		_nfds;

		struct epoll_event events[1024];
		std::vector<struct eventStructTmp *> _structs;

		bool					isServerFd(std::vector<Server *> &Servers, int fdTmp);
		struct eventStructTmp*	createEventStruct(int fd, Server* server, bool serverOrClient);
		void					processEvents(std::vector<Server*> &Servers);
		void					sendResponse(HttpResponse &response, int fdTmp, const strMap& m);
		str						getRequest(int fdTmp);
		void					handleClientRequest(int fd, eventStructTmp *eventStrct);
		void					handleClientConnection(int fd, eventStructTmp *eventStrct);
		void					safeCloseAndDelete(int fd, eventStructTmp* eventStruct);
		bool					checkCGI(str path, Server server);

	public:
		EventPool(std::vector<Server*> &Servers);
		~EventPool();
		void	poolLoop(std::vector<Server*> &Servers);
		void	acceptConnection(int fdTmp, Server *server);


		class socketReadException : public std::exception
		{
			private:
				str message;
			public:
				socketReadException(int fd);
				const char *what() const throw();
				virtual ~socketReadException() throw() {}
		};
		class disconnectedException : public std::exception
		{
			private:
				str message;
			public:
				disconnectedException(int fd);
				const char *what() const throw();
				virtual ~disconnectedException() throw() {}
		};
		class AcceptConnectionException : public std::exception
		{
			private:
				str message;
			public:
			AcceptConnectionException(str const &msg);
				const char *what() const throw();
				virtual ~AcceptConnectionException() throw() {}
		};
};

