/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/12 15:12:38 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <fstream>

const str HttpRequest::saveHeader(const str &request) {
    str::size_type end = request.find("\r\n");

    if (end == str::npos)
		throw badHeaderException("No \\r\\n found at header");
    str line = request.substr(0, end);
    str::size_type separator = line.find(": ");
    if (separator != str::npos) {
        str key = line.substr(0, separator);
        str value = line.substr(separator + 2);
        _header[key] = value;
    }
    if (end + 2 < request.length()) {
        return saveHeader(request.substr(end + 2));
    }
    return request;
}

#define DEFAULT_ERROR "Invalid request line: expected '<METHOD> <PATH> <VERSION>' format"

void HttpRequest::checkHeaderMRP(const str &line) {
	str::size_type end = line.find("\r\n");
    if (end == str::npos)
		throw badHeaderException("No \\r\\n found at status line");
	Logger::log(str("Cheacking Heder: ") + line, INFO);
	if (line.empty())
		throw badHeaderException("Empty Http request received.");
	end = line.find(" ", 0);
	if (end == str::npos)
		throw badHeaderException("No space after method received.");
	str	method = line.substr(0, end);
	_receivedMethod = method;
	if (method == "GET") _type = GET;
	else if (method == "POST") _type = POST;
	else if (method == "OPTIONS") _type = OPTIONS;
	else if (method == "DELETE") _type = DELETE;
	else if (method == "PUT") _type = PUT;
	else
		throw badHeaderException("Bad Method.");
	if (line.at(++end) != '/') // IMPORTANT probar si hay mas espacios
		_badRequest = true;
	size_t path_end = line.find(" ", end);
	if (path_end == str::npos)
		throw badHeaderException(DEFAULT_ERROR);
	_path = line.substr(end, path_end - end);
	if (line.substr(path_end + 1) != "HTTP/1.1\r\n") //aqui no tinene \r\n porque ya lo quitamos en el constructor
		throw badHeaderException("Bad Protocol version");
	//_resource
	//_varCgi
	//_queryString
}

bool HttpRequest::checkResource(Server const &server) {
  _resourceExist = false;
  if (!_location)
    return false;
  str root = _location->getRoot();
  if (root.empty())
    root = server.getRoot();
  std::ifstream resource((root + _resource).c_str());
  _resourceExist = resource.good();
  return resource;
}

Location*	HttpRequest::findLocation(Server* Server) {
	Logger::log(str("Looking for Location: ") + _path, INFO);
	std::vector<Location*> locations = Server->getLocations();
	for (int i = 0 ; i < locations.size(); i++) {
		Logger::log(str("Comparando Location: ") + _locationPath + "con: " + locations[i]->getUrlPath());
		if (_locationPath == locations[i]->getUrlPath()) {
			Logger::log("Location Encontrada", USER);
			return locations[i];
		}
	}
	Logger::log(str("No se encontro location para este recurso: ") + _path + _resource, USER);
	return NULL;
}

bool	HttpRequest::checkAllowMethod()
{
	int method = (_receivedMethod == "GET")		? 0 :
				(_receivedMethod == "POST")		? 1 :
				(_receivedMethod == "DELETE")	? 2 :
				(_receivedMethod == "OPTIONS")	? 3 :
				(_receivedMethod == "PUT")		? 4 :
				-1;
	for (size_t i = 0; i < _location->getMethods().size(); i++) {
		if (_location->getMethods()[i] == method)
			_validMethod = true;
	}
	return _validMethod;
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

void	HttpRequest::autoIndex(Location *loc) {
	if (!loc->getIndex().empty()) {
		_resource = loc->getIndex();
		_resourceExist = true;
	}
	else if (loc->getAutoindex())
		_resourceExist = true;
	else
		_resourceExist = false;
}

void	HttpRequest::envPath(Server* server) {
	strVec		locationPaths = Utils::split(_path, '/');
	strVecIt	it;
	bool isCgi;
	for (it = locationPaths.begin() ; it != locationPaths.end(); ++it) {
		isCgi = false;
		if ((*it).find('.') != str::npos) {
			isCgi = checkIsCgi(it, locationPaths.end(), server);
			if (!isCgi)
				autoIndex(findLocation(server));
		}
		if (!(*it).empty() && !isCgi && (it +1) != locationPaths.end() && *(it +1) != "")
			_locationPath.append("/" + (*it));
		else if (*(it + 1) == "")
			_resourceExist = false;
		else
			_resource.append((*it));
	}
}

HttpRequest::HttpRequest(str request, Server *server) : AHttp(request), _badRequest(false), _validMethod(false), _isValidCgi(false) {
	_location = NULL;
	str::size_type end = request.find("\r\n");
	if (end == str::npos) {
		_badRequest = true; return ;
	}
	const str line = request.substr(0, end + 2);
	try {
		checkHeaderMRP(line);
		envPath(server);
		if(!checkResource(*server))
			return ;
		if(!checkAllowMethod())
			return ;
		_location = findLocation(server);
		_body = saveHeader(request.substr(end));
	} catch(const badHeaderException &e) {
		_badRequest = true;
		Logger::log(e.what(), USER);
	} catch(...) {
		_badRequest = true;
		Logger::log("UNKNOWN FATAL ERROR AT HTTPREQUEST HEADER", ERROR);
	}
}

HttpRequest::~HttpRequest() {}

//Getters
RequestType	HttpRequest::getType() const { return _type; }
bool		HttpRequest::getBadRequest() const { return _badRequest; }
bool		HttpRequest::getResorceExist() const { return _resourceExist; }
bool		HttpRequest::getValidMethod() const { return _validMethod; }
bool		HttpRequest::getIsCgi() const { return _isCgi; }
bool		HttpRequest::getIsValidCgi() const { return _isValidCgi; }
Location*	HttpRequest::getLocation() const { return _location; }

//Setters
void		HttpRequest::setType(RequestType type) { _type = type; }
void		HttpRequest::setBadRequest(bool badRequest) { _badRequest = badRequest; }
void		HttpRequest::setResorceExist(bool resorceExist) { _resourceExist = resorceExist; }
void		HttpRequest::setValidMethod(bool validMethod) { _validMethod = validMethod; }
void		HttpRequest::setIsCgi(bool isCgi) { _isCgi = isCgi; }
void		HttpRequest::setIsValidCgi(bool isValidCgi) { _isValidCgi = isValidCgi; }
void 		HttpRequest::setLocation(Location *location) { _location = location; }

HttpRequest::badHeaderException::badHeaderException(const str &msg) : _msg(msg) {}
const char	*HttpRequest::badHeaderException::what() const throw() { return _msg.c_str(); }
