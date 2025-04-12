/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/10 12:15:35 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <fstream>

bool	HttpRequest::checkValidCgi(strVecIt it) {
	// IMPORTANT poner si es folder
	std::ifstream isValidCgi((*it).c_str());
	if (isValidCgi.good()) {
		_isValidCgi = true;
		return true;
	}
	return false;
}

void	HttpRequest::saveScriptNameAndQueryString(strVecIt it, strVecIt end) {
	size_t infoPos = (*it).find('?');
	if (infoPos != str::npos) {
		if ((it + 1) != end)
			return ; // IMPORTANT add an error here where --> adios.py?algo=algomas (/) algoMas
		_queryString = (*it).substr(infoPos + 1);
		if ((*it).find('.') == str::npos)
			_pathInfo.append("/" + (*it).substr(0, infoPos));
		else
			_resource = (*it).substr(0, infoPos);
		return ;
	}
	if ((*it).find('.') != str::npos)
		_resource = (*it).substr(0);
}

bool	HttpRequest::checkIsCgi(strVecIt it, strVecIt end, Server* server) {
	Location *loc = findLocation(server);
	if (!loc)
		return false;
	std::vector<str> validExtensions = loc->getCgiExtension();
	if (validExtensions.empty())
		return false;
	for (; it != end; it++) {
		if ((*it).find('.') != str::npos && checkValidCgi(it)) {
			saveScriptNameAndQueryString(it, end);
			if ((*it).find('?') == str::npos)
				addPathInfo(it + 1, end);
			_isCgi = true;
			break ;
		}
		_locationPath.append("/" + (*it));
	}

	if (!loc->getCgiEnable())
	return false;
	str extension = (*it).substr((*it).find_last_of('.'));
	bool valid = false;
	for (int i = 0; i < validExtensions.size(); i++)
	if (extension == validExtensions[i])
	valid = true;
	str total_path = server->getRoot();
	if (valid) {
		if (!loc->getRoot().empty())
		total_path.append(loc->getRoot());
	}
	_location = findLocation(server);
	return true;
}

// /hola aaaa bbbbbbbb adios.py?algo=algomas
// /hola aaaa bbbbbbbb adios.py hola adios a?hola=a
// /hola aaaa bbbbbbbb adios.py hola adios a

// we gotta check for those but it should work
// /hola aaaa bbbbbbbb adios.py hola adios a.py?hola=a
// /hola aaaa bbbbbbbb adios.py hola adios a.py
