/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:10 by shurtado          #+#    #+#             */
/*   Updated: 2025/05/05 11:17:44 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "../Utils/Logger.hpp"
#include "ParseLocation.hpp"
#include <cctype>
#include <cstdlib>

Location::Location(const str &path)
    : _uploadEnable(false), _root(path), _autoIndex(false), _cgiEnable(false),
      _bodySize(0) {
  _methods.push_back(GET);
  _methods.push_back(POST);
  _methods.push_back(DELETE);
  _methods.push_back(OPTIONS);
  _methods.push_back(PUT);
  _uploadPath = "";
  _cgiPath = "/usr/bin/python3";
  _root = "";
  _urlPath = path;
}

Location::~Location() {}

std::vector<RequestType> Location::getMethods() const { return this->_methods; }
str Location::getRedirect() const { return this->_redirect; }
bool Location::getUploadEnable() const { return this->_uploadEnable; }
str Location::getRoot() const { return this->_root; }
bool Location::getAutoindex() const { return this->_autoIndex; }
str Location::getIndex() const { return this->_index; }
str Location::getUploadPath() const { return this->_uploadPath; }
bool Location::getCgiEnable() const { return this->_cgiEnable; }
strVec Location::getCgiExtension() const { return this->_cgiExtension; }
str Location::getCgiPath() const { return this->_cgiPath; }
str Location::getRedirectCode() const { return this->_redirect_code; }
int Location::getBodySize() const { return this->_bodySize; }
str Location::getUrlPath() const { return this->_urlPath; }

void Location::setMethods(const str &Methods) {

  strVec allMethods = Utils::split(Methods, ' ');
  std::vector<RequestType> result;
  for (strVec::const_iterator it = allMethods.begin(); it != allMethods.end();
       ++it) {
    if ((*it).empty()) {
      Logger::log("Empty method inside vector.", WARNING);
      continue;
    }
    result.push_back(strToRequest(*it));
  }
  this->_methods = result;
}
void Location::setRedirect(str redirect) { this->_redirect = redirect; }
void Location::setUploadEnable(bool uploadEnable) {
  this->_uploadEnable = uploadEnable;
}
void Location::setRoot(str root) { this->_root = root; }
void Location::setAutoindex(bool autoIndex) { this->_autoIndex = autoIndex; }
void Location::setIndex(str index) { this->_index = index; }
void Location::setUploadPath(str uploadPath) { this->_uploadPath = uploadPath; }
void Location::setCgiEnable(bool cgiEnable) { this->_cgiEnable = cgiEnable; }
void Location::setCgiExtension(str cgiExtension) {
  strVec allExtensions = Utils::split(cgiExtension, ' ');
  strVec result;
  for (strVecIt it = allExtensions.begin(); it != allExtensions.end(); ++it) {
    if ((*it).empty())
      continue;
    if ((*it)[0] != '.') {
      Logger::log(str("Invalid cgi extension:") + *it, WARNING);
      continue;
    }

    result.push_back(*it);
  }
  this->_cgiExtension = result;
}
void Location::setCgiPath(str cgiPath) { this->_cgiPath = cgiPath; }
void Location::setRedirectCode(const str &code) {
  if (code.empty() || code.size() != 3 || code.at(0) != '3' ||
      code.at(1) != '0')
    return;
  if (!std::isdigit(code.at(2)))
    return;
  this->_redirect_code = code;
}
void Location::setBodySize(const str &size) {
  this->_bodySize = std::atoi(size.c_str());
  if (_bodySize == 0)
    throw BadSyntaxLocationBlockException("Error on Body size");
}

void Location::setLocalPath(const str &localPath) { _localPath = localPath; }

const str Location::getLocalPathFromUri(const str &uri) const {
  if (uri.find(_urlPath) == std::string::npos)
    return uri;
  str localPath = uri.substr(_urlPath.size());
  localPath.insert(0, _localPath);
  return localPath;
}

const str Location::getUriFromLocalPath(const str &localPath) const {
  if (localPath.find(_localPath) == std::string::npos)
    return localPath;
  str uri = localPath.substr(_localPath.size());
  uri.insert(0, _urlPath);
  return uri;
}
