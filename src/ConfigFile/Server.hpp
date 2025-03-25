/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:39 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 17:52:10 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef SERVER_HPP
#define SERVER_HPP

#include "Location.hpp"
#include <map>

class Server {
	public:
		Server();
		Server(const str &server);
		Server(const Server &other);
		Server& operator=(const Server &other);
		~Server();

		//Getters
		std::vector<Location>		getLocations() const;
		std::map<int, str>	getErrorPages() const;
		str					getServerName() const;
		str					getHostName() const;
		str							getPort() const;
		str					getRoot() const;
		bool						getIsdefault() const;
		size_t						getBodySize() const;
		bool						getCgiEnable() const;
		str					getCgiExtension() const;
		str					getCgiPath() const;

		//Setters
		void						setLocations(std::vector<Location> locationVector);
		void						setErrorPages(std::map<int, str> errorPages);
		void						setServerName(str serverName);
		void						setHostName(str hostName);
		void						setPort(str port);
		void						setRoot(str root);
		void						setIsdefault(bool isDefault);
		void						setBodySize(size_t bodySize);
		void						setCgiEnable(bool cgiEnable);
		void						setCgiExtension(str cgiExtension);
		void						setCgiPath(str cgiPath);


	private:
		std::vector<Location>		_locations;
		std::map<int, str>	_errorPages;
		str					_serverName;
		str					_hostName;
		str					_port;
		str					_root;
		bool						_isDefault;
		size_t						_bodySize;
		bool						_cgiEnable;
		str					_cgiExtension;
		str					_cgiPath;
};

#endif


