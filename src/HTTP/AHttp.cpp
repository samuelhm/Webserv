/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHttp.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 15:18:16 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "AHttp.hpp"

AHttp::AHttp() {}

AHttp::AHttp(str request) {
	(void)request;
}

AHttp::AHttp(const AHttp &other) { *this = other; }

AHttp& AHttp::operator=(const AHttp &other) {

	if (this != &other) {
		_path = other._path ;
		_header = other._header ;
		_body = other._body ;
	}
	return *this;
}

AHttp::~AHttp() {}

std::map<str, str>& AHttp::get_header() { return _header; }
const str& AHttp::get_body() const { return _body; }
const str& AHttp::get_path() const { return _path; }

