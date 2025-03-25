/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 17:52:10 by shurtado         ###   ########.fr       */
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
		str					getRedirect() const;
		bool						getUploadEnable() const;
		str					getRoot() const;
		bool						getAutoindex() const;
		str					getIndex() const;
		str					getUploadPath() const;

		//Setters
		void						setMethods(std::vector<RequestType> methods);
		void						setRedirect(str redirect);
		void						setUploadEnable(bool uploadEnable);
		void						setRoot(str root);
		void						setAutoindex(bool autoIndex);
		void						setIndex(str index);
		void						setUploadPath(str uploadPath);

	private:
		std::vector<RequestType>	_methods;
		str					_redirect;
		bool						_uploadEnable;
		str					_root;
		bool						_autoIndex;
		str					_index;
		str					_uploadPath;
};

#endif


