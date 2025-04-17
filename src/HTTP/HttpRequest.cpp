/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/17 17:48:31 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <fstream>

HttpRequest::HttpRequest(str request, Server *server)
	: AHttp(request), _badRequest(false), _resourceExists(false), _validMethod(false),
		_isValidCgi(false), _headerTooLarge(false), _redirect(false)
{
	_location = NULL;

	str::size_type end = request.find("\r\n");
	if (end == str::npos) {
		Logger::log("no \\r\\n found!!!", USER);
		_badRequest = true; return ;
	}
	const str line = request.substr(0, end + 2);
	try {
		checkHeaderMRP(line);
		if (!envPath(server)) {
			// check autoindex here	
			return ;
		}
		// por ahora se va
		// if(!checkResource(*server))
		// 	return ;
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
    if (end + 2 < request.length())
        return saveHeader(request.substr(end + 2));
    return request;
}

#define DEFAULT_ERROR "Invalid request line: expected '<METHOD> <PATH> <VERSION>' format"

void HttpRequest::checkHeaderMRP(const str &line) {
	Logger::log(str("Cheacking Heder: ") + line, INFO);
	if (line.empty())
		throw badHeaderException("Empty Http request received.");
	size_t end = line.find(" ", 0);
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
	_uri = line.substr(end, path_end - end);
	if (line.substr(path_end + 1) != "HTTP/1.1\r\n") //aqui no tinene \r\n porque ya lo quitamos en el constructor
		throw badHeaderException("Bad Protocol version");
	//_resource
	//_varCgi
	//_queryString
}

// bool HttpRequest::checkResource(Server const &server) {
//   _resourceExists = false;
//   if (!_location)
//     return false;
//   str root = _location->getRoot();
//   if (root.empty())
//     root = server.getRoot();
//   std::ifstream resource((root + _resource).c_str());
//   _resourceExists = resource.good();
//   return resource;
// }

Location*	HttpRequest::findLocation(Server* Server) {
	Logger::log(str("Looking for Location: ") + _uri, INFO);
	std::vector<Location*> locations = Server->getLocations();
	for (std::size_t i = 0 ; i < locations.size(); i++) {
		Logger::log(str("Comparando Location: ") + _locationUri + "con: " + locations[i]->getUrlPath());
		if (_locationUri == locations[i]->getUrlPath()) {
			Logger::log("Location Encontrada", USER);
			return locations[i];
		}
	}
	Logger::log(str("No se encontro location para este recurso: ") + _uri + _resource, USER);
	return NULL;
}

Location*	HttpRequest::findLocation(Server* Server, const str &uri) {
	Logger::log(str("Looking for Location: ") + uri, INFO);
	std::vector<Location*> locations = Server->getLocations();
	for (std::size_t i = 0 ; i < locations.size(); i++) {
		Logger::log(str("Comparando Location: ") + uri + "con: " + locations[i]->getUrlPath());
		if (uri == locations[i]->getUrlPath()) {
			Logger::log("Location Encontrada", USER);
			return locations[i];
		}
	}
	Logger::log(str("No se encontro location para este recurso: ") + _uri + _resource, USER);
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

void	HttpRequest::autoIndex(Location *loc) {
	if (!loc->getIndex().empty()) {
		_resource = loc->getIndex();
		_resourceExists = true;
	}
	else if (loc->getAutoindex())
		_resourceExists = true;
	else
		_resourceExists = false;
}

bool	HttpRequest::envPath(Server* server) {
	strVec		locationUris = Utils::split(_uri, '/');
	strVecIt	it;

	if (!saveUri(locationUris.begin(), locationUris.end(), server))
		return false;
	return true;
}

HttpRequest::~HttpRequest() {}

//Getters
RequestType	HttpRequest::getType() const { return _type; }
bool		HttpRequest::getBadRequest() const { return _badRequest; }
bool		HttpRequest::getResourceExists() const { return _resourceExists; }
bool		HttpRequest::getValidMethod() const { return _validMethod; }
bool		HttpRequest::getIsCgi() const { return _isCgi; }
bool		HttpRequest::getIsValidCgi() const { return _isValidCgi; }
bool		HttpRequest::getHeaderTooLarge() const { return _headerTooLarge; }
Location*	HttpRequest::getLocation() const { return _location; }
str			HttpRequest::getReceivedMethod() const { return _receivedMethod; }
str			HttpRequest::getResource() const { return _resource; }
str			HttpRequest::getLocalPathResource() const { return _localPathResource; }
str			HttpRequest::getLocationUri() const { return _locationUri; }
str			HttpRequest::getQueryString() const { return _queryString; }
str			HttpRequest::getPathInfo() const { return _pathInfo; }
bool		HttpRequest::getRedirect() const { return _redirect; }

//Setters
void		HttpRequest::setType(RequestType type) { _type = type; }
void		HttpRequest::setBadRequest(bool badRequest) { _badRequest = badRequest; }
void		HttpRequest::setResorceExist(bool resorceExist) { _resourceExists = resorceExist; }
void		HttpRequest::setValidMethod(bool validMethod) { _validMethod = validMethod; }
void		HttpRequest::setIsCgi(bool isCgi) { _isCgi = isCgi; }
void		HttpRequest::setIsValidCgi(bool isValidCgi) { _isValidCgi = isValidCgi; }
void 		HttpRequest::setLocation(Location *location) { _location = location; }

HttpRequest::badHeaderException::badHeaderException(const str &msg) : _msg(msg) {}
const char	*HttpRequest::badHeaderException::what() const throw() { return _msg.c_str(); }
