/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/02 20:18:13 by shurtado         ###   ########.fr       */
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
#include <sys/wait.h>
#include <sys/types.h>
#include "../ConfigFile/Server.hpp"
#include "../HTTP/HttpRequest.hpp"
#include "../HTTP/HttpResponse.hpp"

extern volatile sig_atomic_t epollRun;

class Server;
class HttpResponse;
class HttpRequest;

struct cgiContext
{
	int		pipeIn;
	int 	pipeOut;
	pid_t	cgiPid;
	str		cgiWrite;
	str		cgiRead;
	size_t	writeOffset;
	bool	HeadersParsed;
};

struct eventStructTmp
{
	Server 		*server;
	int			client_fd;
	EventType	eventType;
	str			content;
	size_t		offset;

	//chunks
	bool		headerParsed;
	bool		isChunked;
	size_t		contentLength;
	str			bodyDecoded;

	//cgi
	struct cgiContext cgiData;
};

class EventPool {
	private:
		int		_pollFd;
		int		_nfds;

		struct epoll_event events[1024];
		std::vector<struct eventStructTmp *> _structs;

		struct eventStructTmp*	createEventStruct(int fd, Server* server, EventType eventType);
		void					processEvents();
		void					saveResponse(HttpResponse &response, eventStructTmp *eventStrct);
		bool					getRequest(int socketFd, eventStructTmp *eventstrct);
		void					handleClientRequest(int fd, eventStructTmp *eventStrct);
		void					handleClientConnection(int fd, eventStructTmp *eventStrct);
		bool					handleClientWrite(int fd, eventStructTmp *eventStrct);
		void					safeCloseAndDelete(int fd, eventStructTmp* eventStruct);
		HttpResponse			stablishResponse(HttpRequest &request, Server *server, eventStructTmp *eventStrct);
		bool					headerTooLarge(str const &request, int &errorCode);
		bool					setContentLength(eventStructTmp* eventstrct, int &content_lenght);
		void					checkBodySize(eventStructTmp* eventstrct);
		void					parseHeader(eventStructTmp* eventstrct);
		bool					processChunk(eventStructTmp* eventstrct, size_t &headerEnd);
		void					handleCgiWrite(int fd, eventStructTmp* eventstrct);
		void					handleCgiRead(int fd, eventStructTmp* eventstrct);
		int						setFd(eventStructTmp *eventStrct);

	public:
		EventPool(std::vector<Server*> &Servers);
		~EventPool();
		void	poolLoop();
		void	acceptConnection(int fdTmp, Server *server);
		int		getPollFd();


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
		class HttpException : public std::exception
		{
		private:
			int _errorCode;
			str _message;
		public:
			HttpException(int errorCode, const str& extra = "");
			virtual const char *what() const throw();
			int getErrorCode() const throw();
			virtual ~HttpException() throw();
		};
};
