/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 11:43:00 by shurtado         ###   ########.fr       */
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

	private:
		std::vector<RequestType>	_methods;
		std::string					_redirect;
		bool						_uploadEnable;
		std::string					_root;
		bool						_autoindex;
		std::string					_index;
		std::string					_uploadPath;
};

#endif


