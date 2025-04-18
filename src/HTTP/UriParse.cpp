/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/18 13:17:46 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

bool	HttpRequest::asignBoolsCgi(str tmp, const strVec vec) {
	str				localPathResource = _localPathResource + str("/") + tmp;
	// const strVec	vec = loc->getCgiExtension();
	str 			extension = tmp.substr(tmp.find_last_of('.'));

	if (vec.empty())
		return false;
	for (std::size_t i = 0; i < vec.size(); i++) {
		if (extension == vec[i].c_str()) {
			_isCgi = true;
			_localPathResource = localPathResource;
			_resourceExists = true;
			_isValidCgi = true;
			return true;
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
			while ((it + 1) != end) {
				it++;
				_queryString.append("/" + (*it));
			}
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

bool isRegularFile(const char* path) {
	struct stat		path_stat;
	std::ifstream	isValidFile(path); // IMPORTANT: if leaks free "path"

    if (isValidFile.is_open() && stat(path, &path_stat) == 0)
        return S_ISREG(path_stat.st_mode);
    return false;
}

bool	HttpRequest::justABar(Server* server) {
	_locationUri = "/";
	_location = findLocation(server, _locationUri);
	if (!_location)
		return false;
	_localPathResource = server->getRoot();
	if (!_location->getRoot().empty())
		_localPathResource.append(_location->getRoot());
	_resource = _location->getIndex();
	if (_resource.empty()) {
		if(!_location->getAutoindex()) {
			return false;
		}
		else {
			_resourceExists = true;
			return true;
		}
	}
	_localPathResource.append("/" + _resource);
	if (isRegularFile(_localPathResource.c_str())) {
		_resourceExists = true;
		return true;
	}
	return false;
}

strVecIt	HttpRequest::findFile(Server* server, strVecIt it, strVecIt end) {
	str		tmpSrcPath;

	for (; it != end; it++) {
		_locationUri.append("/" + (*it));
		_location = findLocation(server, _locationUri);
		if (!_location)
			continue ;
		tmpSrcPath = server->getRoot();
		if (!_location->getRoot().empty())
			tmpSrcPath.append(_location->getRoot());
		str tmp = (*(it + 1));
		if (tmp.find('?') != str::npos)
			tmp = tmp.substr(0, tmp.find('?'));
		tmpSrcPath.append("/" + tmp);
		if (isRegularFile(tmpSrcPath.c_str())) {
			it++;
			break ;
		}
	}
	return it;
}

bool	HttpRequest::saveUri(strVecIt it, strVecIt end, Server* server) {
	if (_uri == "/" || _uri.empty())
		return justABar(server);
	it = findFile(server, ++it, end);
	if (!_location)
		return false;

	_localPathResource = server->getRoot();
	if (!_location->getRoot().empty())
		_localPathResource.append(_location->getRoot());

	str total_path = server->getRoot();
	for (; it != end; it++) {
		str			tmp = (*it);
		str			localPathResource = _localPathResource + str("/") + (*it).c_str();

		if (tmp.find('?') != str::npos)
			tmp = tmp.substr(0, tmp.find('?'));
		if ((*it).find('.') != str::npos && isRegularFile(localPathResource.c_str())) {
			saveScriptNameAndQueryString(it, end);
			if ((*it).find('?') == str::npos)
				addPathInfo(it + 1, end);
			asignBoolsCgi(tmp, _location->getCgiExtension());
			if (_isCgi)
				break ;
		}
		if (isRegularFile(localPathResource.c_str())) {
			_resource = (*it).substr(0);
			_localPathResource = localPathResource;
			_resourceExists = true;
			break ;
		}
		_locationUri.append("/" + (*it));
	}
	return true;
}
