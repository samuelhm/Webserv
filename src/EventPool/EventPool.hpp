/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventPool.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/27 14:47:11 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/30 15:18:10 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef EVENTPOOL_HPP
#define EVENTPOOL_HPP

#include <vector>
#include <sys/epoll.h>
#include <exception>
#include <cerrno>
#include <cstring>
#include <sstream>
#include "../ConfigFile/Server.hpp"
#include "../HTTP/HttpRequest.hpp"
#include "../HTTP/HttpResponse.hpp"

class EventPool {
	private:
		int		_pollFd;
		int		_nfds;
		struct epoll_event events[1024];

		EventPool(const EventPool &other);
		EventPool& operator=(const EventPool &other);
		bool isServerFd(std::vector<Server *> &Servers, int fdTmp);

	public:
		EventPool(std::vector<Server*> &Servers);
		~EventPool();
		void	poolLoop(std::vector<Server*> &Servers);
		str		getRequest(int fdTmp);
		void	sendResponse(HttpResponse &response, int fdTmp);
		void	acceptConnection(int fdTmp);


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
};

#endif
