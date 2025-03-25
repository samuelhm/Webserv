/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:10 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 16:44:51 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(const std::string &serverName) {

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
std::string					Location::getRedirect() const { return this->_redirect; }
bool						Location::getUploadEnable() const { return this->_uploadEnable; }
std::string					Location::getRoot() const { return this->_root; }
bool						Location::getAutoindex() const { return this->_autoIndex; }
std::string					Location::getIndex() const { return this->_index; }
std::string					Location::getUploadPath() const { return this->_uploadPath; }

//Setters
void						Location::setMethods(std::vector<RequestType> methods) { this->_methods = methods; }
void						Location::setRedirect(std::string redirect) { this->_redirect = redirect; }
void						Location::setUploadEnable(bool uploadEnable) { this->_uploadEnable = uploadEnable; }
void						Location::setRoot(std::string root) { this->_root = root; }
void						Location::setAutoindex(bool autoIndex) { this->_autoIndex = autoIndex; }
void						Location::setIndex(std::string index) { this->_index = index; }
void						Location::setUploadPath(std::string uploadPath) { this->_uploadPath = uploadPath; }
