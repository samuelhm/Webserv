/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:50:39 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 16:26:17 by shurtado         ###   ########.fr       */
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

		//Getters
		std::vector<Location>		getLocations() const;
		std::map<int, std::string>	getErrorPages() const;
		std::string					getServerName() const;
		std::string					getHostName() const;
		int							getPort() const;
		std::string					getRoot() const;
		bool						getIsdefault() const;
		size_t						getBodySize() const;
		bool						getCgiEnable() const;
		std::string					getCgiExtension() const;
		std::string					getCgiPath() const;

		//Setters
		void						setLocations(std::vector<Location> locationVector);
		void						setErrorPages(std::map<int, std::string> errorPages);
		void						setServerName(std::string serverName);
		void						setHostName(std::string hostName);
		void						setPort(int port);
		void						setRoot(std::string root);
		void						setIsdefault(bool isDefault);
		void						setBodySize(size_t bodySize);
		void						setCgiEnable(bool cgiEnable);
		void						setCgiExtension(std::string cgiExtension);
		void						setCgiPath(std::string cgiPath);


	private:
		std::vector<Location>		_locations;
		std::map<int, std::string>	_errorPages;
		std::string					_serverName;
		std::string					_hostName;
		int							_port;
		std::string					_root;
		bool						_isDefault;
		size_t						_bodySize;
		bool						_cgiEnable;
		std::string					_cgiExtension;
		std::string					_cgiPath;
};

#endif


