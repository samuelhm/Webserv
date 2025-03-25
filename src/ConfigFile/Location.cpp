/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:10 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 17:52:10 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(const str &serverName) {

	_methods.push_back(GET);
	_uploadEnable = false;
	_autoIndex = false;
	_index = "index.html";
	_uploadPath = "./" + serverName;
}

Location::Location(const Location &other) {
	*this = other;
}

Location& Location::operator=(const Location &other) {
	if (this != &other) {
		this->_methods = other._methods;
		this->_redirect = other._redirect;
		this->_uploadEnable = other._uploadEnable;
		this->_root = other._root;
		this->_autoIndex = other._autoIndex;
		this->_index = other._index;
		this->_uploadPath = other._uploadPath;
	}
	return *this;
}

Location::~Location() {}

//Getters
std::vector<RequestType>	Location::getMethods() const { return this->_methods; }
str					Location::getRedirect() const { return this->_redirect; }
bool						Location::getUploadEnable() const { return this->_uploadEnable; }
str					Location::getRoot() const { return this->_root; }
bool						Location::getAutoindex() const { return this->_autoIndex; }
str					Location::getIndex() const { return this->_index; }
str					Location::getUploadPath() const { return this->_uploadPath; }

//Setters
void						Location::setMethods(std::vector<RequestType> methods) { this->_methods = methods; }
void						Location::setRedirect(str redirect) { this->_redirect = redirect; }
void						Location::setUploadEnable(bool uploadEnable) { this->_uploadEnable = uploadEnable; }
void						Location::setRoot(str root) { this->_root = root; }
void						Location::setAutoindex(bool autoIndex) { this->_autoIndex = autoIndex; }
void						Location::setIndex(str index) { this->_index = index; }
void						Location::setUploadPath(str uploadPath) { this->_uploadPath = uploadPath; }
