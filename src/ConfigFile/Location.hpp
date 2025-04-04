/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/02 18:10:45 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <vector>
#include <string>
#include "../WebSrv.hpp"

class Location {
	public:
		Location(const str &serverName);
		Location(const Location &other);
		Location& operator=(const Location &other);
		~Location();

		//Getters
		std::vector<RequestType>	getMethods() const;
		str							getRedirect() const;
		bool						getUploadEnable() const;
		str							getRoot() const;
		bool						getAutoindex() const;
		str							getIndex() const;
		str							getUploadPath() const;
		bool						getCgiEnable() const;
		str							getCgiExtension() const;
		str							getCgiPath() const;

		//Setters
		void						setMethods(std::vector<RequestType> methods);
		void						setRedirect(str redirect);
		void						setUploadEnable(bool uploadEnable);
		void						setRoot(str root);
		void						setAutoindex(bool autoIndex);
		void						setIndex(str index);
		void						setUploadPath(str uploadPath);
		void						setCgiEnable(bool cgiEnable);
		void						setCgiExtension(str cgiExtension);
		void						setCgiPath(str cgiPath);

	private:
		std::vector<RequestType>	_methods;
		str							_redirect;
		bool						_uploadEnable;
		str							_root;
		bool						_autoIndex;
		str							_index;
		str							_uploadPath;
		bool						_cgiEnable;
		str							_cgiExtension;
		str							_cgiPath;
};

#endif
