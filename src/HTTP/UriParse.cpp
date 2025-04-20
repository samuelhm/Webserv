/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UriParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/19 13:15:49 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"

str	HttpRequest::addPathInfo(str afterSrc) {
	size_t		breakPoint = afterSrc.find('?');
	str			tmp;

	if (breakPoint != str::npos) {
		tmp = afterSrc.substr(0, breakPoint);
		if (tmp.find('/') != str::npos) {
			_pathInfo = tmp;
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

void	HttpRequest::parseResource() {
	str				afterSrc;
	strVec		extensions = _location->getCgiExtension();
	size_t		extStart = str::npos;
	str			tmp;
	str			checkResource = _resource;
	for (size_t i = 0; i < extensions.size(); i++)
	{
		extStart = checkResource.find(extensions[i]);
		while (extStart != str::npos)
		{
			tmp = checkResource.substr(0, extStart + extensions[i].size());
			if (checkResource[tmp.size()] == '/' || checkResource[tmp.size()]  == '?' || (checkResource.c_str()[tmp.size()] == '\0')) {
				str		resTmp = _localPathResource.substr(0, _localPathResource.find(tmp) + tmp.size());
				if (!Utils::isDirectory(resTmp) && _location->getCgiEnable()) {
					checkResource = checkResource.substr(0, tmp.size());
					_isCgi = true;
					afterSrc = _resource.substr((_resource.find(checkResource) + checkResource.size()));
					_resource = _resource.substr(0, (_resource.find(checkResource) + checkResource.size()));
					tmp = addPathInfo(afterSrc);
					if (!tmp.empty())
						_queryString = tmp.substr(1);
					return ;
				}
			}
			checkResource = checkResource.substr(_resource.find(tmp) + tmp.size() + 1);
			extStart = checkResource.find(extensions[i]);
		}
	}
	return ;
}
