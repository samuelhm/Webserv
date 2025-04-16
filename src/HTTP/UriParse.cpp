/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/16 17:55:48 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <fstream>
#include <dirent.h>

bool	HttpRequest::checkValidCgi(strVecIt it, Location *loc) {
	str				localPathResource = _localPathResource + str("/") + (*it).c_str();
	DIR*			dir = opendir(localPathResource.c_str());
	std::ifstream	isValidCgi(localPathResource.c_str());
	const strVec	vec = loc->getCgiExtension();
	str 			extension = (*it).substr((*it).find_last_of('.'));

	for (std::size_t i = 0; i < vec.size(); i++) {
		if (extension == vec[i].c_str()) {
			_isCgi = true;
			if (isValidCgi.good() && !dir) {
				_localPathResource = localPathResource;
				_resourceExist = true;
				_isValidCgi = true;
				return true;
			}
		}
	}
	return false;
}

void	HttpRequest::saveScriptNameAndQueryString(strVecIt it, strVecIt end) {
	size_t infoPos = (*it).find('?');
	if (infoPos != str::npos) {
		_queryString = (*it).substr(infoPos + 1);
		if (_resource.empty())
			_resource = (*it).substr(0, infoPos);
		if ((*it).find('.') == str::npos)
			_pathInfo.append("/" + (*it).substr(0, infoPos));
		if ((it + 1) != end) {
			for (; it != end; it++)
				_queryString.append("/" + (*it));
		}
		return ;
	}
	else
		_resource = (*it).substr(0);
}

void	HttpRequest::addPathInfo(strVecIt it, strVecIt end) {
	for (; it != end; it++) {
		if ((*it).find('?') != str::npos) {
			saveScriptNameAndQueryString(it, end);
			break ;
		}
		_pathInfo.append("/" + (*it));
	}
}

bool	HttpRequest::saveUri(strVecIt it, strVecIt end, Server* server) {
	Location *loc = findLocation(server, _locationUri);
	if (!loc)
		return false;

	_localPathResource = server->getRoot();
	if (!loc->getRoot().empty())
		_localPathResource.append(loc->getRoot());
	strVec validExtensions = loc->getCgiExtension();
	if (validExtensions.empty())
		return false;
	str total_path = server->getRoot();
	for (; it != end; it++) {
		if ((*it).find('.') != str::npos && checkValidCgi(it, loc)) {
			saveScriptNameAndQueryString(it, end);
			if ((*it).find('?') == str::npos)
				addPathInfo(it + 1, end);
			break ;
		}
		str	localPathResource = _localPathResource + str("/") + (*it).c_str();
		if (std::ifstream(localPathResource.c_str()).good() && !opendir(localPathResource.c_str())) {
			_resource = (*it).substr(0);
			_localPathResource = localPathResource;
			_resourceExist = true;
			break ;
		}
		_locationUri.append("/" + (*it));
	}
	if (!_isCgi || !loc->getCgiEnable())
		return false;
	_location = findLocation(server);
	return true;
}

// /hola aaaa bbbbbbbb adios.py?algo=algomas
// /hola aaaa bbbbbbbb adios.py hola adios a?hola=a
// /hola aaaa bbbbbbbb adios.py hola adios a

// we gotta check for those but it should work
// /hola aaaa bbbbbbbb adios.py hola adios a.py?hola=a
// /hola aaaa bbbbbbbb adios.py hola adios a.py
