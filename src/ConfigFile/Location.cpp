/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:10 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 10:16:20 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "ParseLocation.hpp"

Location::Location(const str &serverName, const str &path)
  : _uploadEnable(false), _root(path), _autoIndex(false), _cgiEnable(false)
{
	_methods.push_back(GET);
	_methods.push_back(POST);
	_methods.push_back(DELETE);
	_methods.push_back(OPTIONS);
	_methods.push_back(PUT);
	_index = "index.html";
	_uploadPath = "./" + serverName + "/";
	_cgiPath = "/usr/bin/python3";
	_root = "";
	_urlPath = path;
}

Location::~Location() {}

//Getters
std::vector<RequestType>	Location::getMethods() const { return this->_methods; }
str							Location::getRedirect() const { return this->_redirect; }
bool						Location::getUploadEnable() const { return this->_uploadEnable; }
str							Location::getRoot() const { return this->_root; }
bool						Location::getAutoindex() const { return this->_autoIndex; }
str							Location::getIndex() const { return this->_index; }
str							Location::getUploadPath() const { return this->_uploadPath; }
bool						Location::getCgiEnable() const { return this->_cgiEnable; }
std::vector<str>			Location::getCgiExtension() const { return this->_cgiExtension; }
str							Location::getCgiPath() const { return this->_cgiPath; }
str							Location::getRedirectCode() const { return this->_redirect_code; }
int							Location::getBodySize() const {return this->_bodySize; }
str							Location::getUrlPath() const {return this->_urlPath; }

//Setters
void						Location::setMethods(const str &Methods) {

	std::vector<str> allMethods = Utils::split(Methods, ' ');
	std::vector<RequestType> result;
	for (std::vector<str>::const_iterator it = allMethods.begin(); it != allMethods.end(); ++it)
	{
		if ((*it).empty())
		{
			Logger::log("Empty method inside vector.", WARNING);
			continue;
		}
		result.push_back(strToRequest(*it));
	}
	this->_methods = result;
}
void	Location::setRedirect(str redirect) { this->_redirect = redirect; }
void	Location::setUploadEnable(bool uploadEnable) { this->_uploadEnable = uploadEnable; }
void	Location::setRoot(str root) { this->_root = root; }
void	Location::setAutoindex(bool autoIndex) { this->_autoIndex = autoIndex; }
void	Location::setIndex(str index) { this->_index = index; }
void	Location::setUploadPath(str uploadPath) { this->_uploadPath = uploadPath; }
void	Location::setCgiEnable(bool cgiEnable) {this->_cgiEnable = cgiEnable;}
void	Location::setCgiExtension(str cgiExtension) {
	std::vector<str> allExtensions = Utils::split(cgiExtension, ' ');
	std::vector<str> result;
	for (std::vector<str>::iterator it = allExtensions.begin(); it != allExtensions.end(); ++it)
	{
		if ((*it).empty())
			continue;
		if ((*it)[0] != '.')
		{
			Logger::log(str("Invalid cgi extension:") + *it, WARNING);
			continue;
		}

		result.push_back(*it);
	}
	this->_cgiExtension = result;
}
void	Location::setCgiPath(str cgiPath) {this->_cgiPath = cgiPath;}
void	Location::setRedirectCode(const str &code) {this->_redirect_code = code; }
void	Location::setBodySize(const str &size){
			this->_bodySize = std::atoi(size.c_str());
			if (_bodySize == 0)
				throw BadSyntaxLocationBlockException("Error on Body size");
		}
