/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/20 19:49:33 by shurtado         ###   ########.fr       */
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
	Server 		*server;
	int			client_fd;
	EventType	eventType;
	str			content;
	size_t		offset;
};

class EventPool {
	private:
		int		_pollFd;
		int		_nfds;

		struct epoll_event events[1024];
		std::vector<struct eventStructTmp *> _structs;

		bool					isServerFd(std::vector<Server *> &Servers, int fdTmp);
		struct eventStructTmp*	createEventStruct(int fd, Server* server, EventType eventType);
		void					processEvents();
		void					saveResponse(HttpResponse &response, eventStructTmp *eventStrct);
		bool					getRequest(int socketFd, eventStructTmp *eventstrct);
		void					handleClientRequest(int fd, eventStructTmp *eventStrct);
		void					handleClientConnection(int fd, eventStructTmp *eventStrct);
		bool					handleClientWrite(int fd, eventStructTmp *eventStrct);
		void					safeCloseAndDelete(int fd, eventStructTmp* eventStruct);
		HttpResponse			stablishResponse(HttpRequest &request, Server *server);
		bool					headerTooLarge(str const &request, int &errorCode);
		bool					setContentLength(eventStructTmp* eventstrct, int &content_lenght);
		bool					checkBodySize(eventStructTmp* eventstrct, int &errorCode);



	public:
		EventPool(std::vector<Server*> &Servers);
		~EventPool();
		void	poolLoop();
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
		class headerTooLargeException : public std::exception
		{
			private:
				const str message;
			public:
				headerTooLargeException(int fd);
				const char *what() const throw();
				virtual ~headerTooLargeException() throw() {}
		};
};
