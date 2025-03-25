/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 16:42:36 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <vector>
#include <string>
#include "../WebSrv.hpp"

class Location {
	public:
		Location(const std::string &serverName);
		Location(const Location &other);
		Location& operator=(const Location &other);
		~Location();

		//Getters
		std::vector<RequestType>	getMethods() const;
		std::string					getRedirect() const;
		bool						getUploadEnable() const;
		std::string					getRoot() const;
		bool						getAutoindex() const;
		std::string					getIndex() const;
		std::string					getUploadPath() const;

		//Setters
		void						setMethods(std::vector<RequestType> methods);
		void						setRedirect(std::string redirect);
		void						setUploadEnable(bool uploadEnable);
		void						setRoot(std::string root);
		void						setAutoindex(bool autoIndex);
		void						setIndex(std::string index);
		void						setUploadPath(std::string uploadPath);

	private:
		std::vector<RequestType>	_methods;
		std::string					_redirect;
		bool						_uploadEnable;
		std::string					_root;
		bool						_autoIndex;
		std::string					_index;
		std::string					_uploadPath;
};

#endif


