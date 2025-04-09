/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:39 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 10:23:18 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "Location.hpp"
#include "ParseConfig.hpp"
#include <map>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h> // fcntl, F_SETFL, O_NONBLOCK
#include <string.h> // memset
#include <iostream> // cout, endl
#include <unistd.h> // close
#include <stdio.h> // perror

class Location;

class Server {
	private:
	std::vector<Location*>		_locations;
	std::map<int, str>			_errorPages;
	str							_serverName;
	str							_hostName;
	str							_port;
	str							_root;
	bool						_isDefault;
	size_t						_bodySize;

	//SocketUp
	int							_serverFd;
	int							_reuseOption;
	struct addrinfo*			_response;
	struct addrinfo				_hints;

	const str &createErrorPage(const str &error, const str &msg);

	public:
		Server();
		~Server();
		bool	operator==(const Server &other);

		//Methods
		void	socketUp();
		bool	locationExist(Location &loc) const;
		void	setListenValue(const str &value);

		//Getters
		std::vector<Location *>&	getLocations();
		const str &					getErrorPage(int error);
		str							getServerName() const;
		str							getHostName() const;
		str							getPort() const;
		str							getRoot() const;
		bool						getIsdefault() const;
		size_t						getBodySize() const;
		int							getServerFd() const;

		//Setters
		void						setLocations(std::vector<Location*> locationVector);
		void						setErrorPages(int error, const str &page);
		void						setServerName(str serverName);
		void						setHostName(str hostName);
		void						setPort(str port);
		void						setRoot(str root);
		void						setIsdefault(bool isDefault);
		void						setBodySize(size_t bodySize);

};

#endif
