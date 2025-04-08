/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 01:22:02 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef EVENTPOOL_HPP
#define EVENTPOOL_HPP

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

		EventPool(const EventPool &other);
		EventPool& operator=(const EventPool &other);
		bool isServerFd(std::vector<Server *> &Servers, int fdTmp);
		struct eventStructTmp* createEventStruct(int fd, Server* server, bool serverOrClient);

		public:
		EventPool(std::vector<Server*> &Servers);
		~EventPool();
		void	poolLoop(std::vector<Server*> &Servers);
		str		getRequest(int fdTmp);
		void	sendResponse(HttpResponse &response, int fdTmp, const std::map<str, str>& m);
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

#endif
