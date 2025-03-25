/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:39 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 12:17:13 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "Location.hpp"
#include <map>

class Server {
	public:
		Server();
		Server(const std::string &server);
		Server(const Server &other);
		Server& operator=(const Server &other);
		~Server();
		std::string	getHostName() const;
		int			getPort() const;

	private:
		std::vector<Location>		_locations;
		std::map<int, std::string>	_errorPages;
		std::string					_serverName;
		std::string					_hostName;
		int							_port;
		std::string					_root;
		bool						_isdefault;
		size_t						_bodySize;
		bool						_cgiEnable;
		std::string					_cgiExtension;
		std::string					_cgiPath;
};

#endif


