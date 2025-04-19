/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/19 12:59:47 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

bool	HttpRequest::checkValidCgi() {
	str				localPathResource = _localPathResource + str("/") + _resource;
	const strVec	vec = _location->getCgiExtension();
	str 			extension = _resource.substr(_resource.find_last_of('.'));

	for (std::size_t i = 0; i < vec.size(); i++) {
		if (extension == vec[i].c_str()) {
			_localPathResource = localPathResource; // CHECK do I do this?
			_isCgi = true;
			_isValidCgi = true;
			return true;
		}
	}
	return false;
}

str	HttpRequest::addPathInfo(str afterSrc) {
	size_t		breakPoint = afterSrc.find('?');
	str			tmp;

	if (breakPoint != str::npos) {
		tmp = afterSrc.substr(0, breakPoint);
		if (tmp.find('/') != str::npos) {
			_pathInfo = tmp; // IMPORTANT check if it saves the "?" or not
			return afterSrc.substr(breakPoint);
		}
		return afterSrc;
	}
	if (afterSrc.find('/') != str::npos) {
		_pathInfo = afterSrc;
		return "";
	}
	return "";
}

bool	HttpRequest::saveUri() {	
	str				afterSrc;
	size_t			breakPoint = _resource.find('?');
	
	if (breakPoint == str::npos)
		breakPoint = _resource.find('/');
	if (breakPoint != str::npos) {
		afterSrc = _resource.substr(breakPoint);
		_resource = _resource.substr(0, breakPoint);
	}
	if (!_location->getCgiEnable() || _resource.find('.') == str::npos || !checkValidCgi())
		return false;
	if (afterSrc.empty())
		return true;
	str		tmp = addPathInfo(afterSrc);
	if (!tmp.empty())
		_queryString = tmp.substr(1); 
	return true;
}
