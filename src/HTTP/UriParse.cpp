/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/18 11:01:48 by erigonza         ###   ########.fr       */
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

bool	HttpRequest::checkValidCgi(str tmp, Location *loc) {
	str				localPathResource = _localPathResource + str("/") + tmp;
	DIR*			dir = opendir(localPathResource.c_str());
	std::ifstream	isValidCgi(localPathResource.c_str());
	const strVec	vec = loc->getCgiExtension();
	str 			extension = tmp.substr(tmp.find_last_of('.'));

	for (std::size_t i = 0; i < vec.size(); i++) {
		if (extension == vec[i].c_str()) {
			_isCgi = true;
			if (isValidCgi.good() && !dir) {
				_localPathResource = localPathResource;
				_resourceExists = true;
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

bool isRegularFile(const char* path) {
    struct stat path_stat;
    if (stat(path, &path_stat) == 0) {
        return S_ISREG(path_stat.st_mode);
    }
    return false;
}

bool	HttpRequest::saveUri(strVecIt it, strVecIt end, Server* server) {
	str			tmpSrcPath;
	
	if (_uri == "/" || _uri.empty()) {
		_locationUri = "/";
		_location = findLocation(server, _locationUri);
		if (!_location)
			return false;
		tmpSrcPath = server->getRoot();
		if (!_location->getRoot().empty())
			tmpSrcPath.append(_location->getRoot());
		_resource = _location->getIndex();
		if (_resource.empty())
			return false;
		tmpSrcPath.append("/" + _resource);
		std::ifstream isValidCgi(tmpSrcPath.c_str());
		if (isValidCgi.is_open() && isRegularFile(tmpSrcPath.c_str())) {
			_resourceExists = true;
			return true;
		}
		return false;
	}
	it++;
	for (; it != end; it++) {
		_locationUri.append("/" + (*it));
		_location = findLocation(server, _locationUri);
		if (!_location)
			continue;
		tmpSrcPath = server->getRoot();
		if (!_location->getRoot().empty())
			tmpSrcPath.append(_location->getRoot());
		str tmp = (*(it + 1));
		if (tmp.find('?') != str::npos)
			tmp = tmp.substr(0, tmp.find('?'));
		tmpSrcPath.append("/" + tmp);
		std::ifstream isValidCgi(tmpSrcPath.c_str());
		if (isValidCgi.is_open() && isRegularFile(tmpSrcPath.c_str())) {
			it++;
			break ;
		}
	}
	if (!_location)
		return false;
	_localPathResource = server->getRoot();
	if (!_location->getRoot().empty())
		_localPathResource.append(_location->getRoot());
	strVec validExtensions = _location->getCgiExtension();
	if (validExtensions.empty())
		return true;
	str total_path = server->getRoot();
	for (; it != end; it++) {
		str tmp = (*it);
		if (tmp.find('?') != str::npos)
			tmp = tmp.substr(0, tmp.find('?'));
		if ((*it).find('.') != str::npos && checkValidCgi(tmp, _location)) {
			saveScriptNameAndQueryString(it, end);
			if ((*it).find('?') == str::npos)
				addPathInfo(it + 1, end);
			break ;
		}
		str	localPathResource = _localPathResource + str("/") + (*it).c_str();
		if (std::ifstream(localPathResource.c_str()).good() && !opendir(localPathResource.c_str())) {
			_resource = (*it).substr(0);
			_localPathResource = localPathResource;
			_resourceExists = true;
			break ;
		}
		_locationUri.append("/" + (*it));
	}
	return true;
}
