/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/19 13:16:54 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include <sys/stat.h>

HttpRequest::HttpRequest(str request, Server *server)
	: AHttp(request), _badRequest(false), _resourceExists(false), _validMethod(false), _isCgi(false),
		_redirect(""), _autoIndex("")
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
		_location = findLocation(server);
		if (!_location)
			return ;
		size_t length = _location->getUrlPath().size();
		if (length != 1)
			length++;
		_resource = _uri.substr(length);
		_localPathResource.append(server->getRoot());
		_localPathResource.append(_location->getRoot());
		_localPathResource.append("/" + _resource);
		if (Utils::isDirectory(_localPathResource)) {
			if (_uri.at(_uri.length() - 1) != '/') {
				_redirect = _uri.append("/");
				return ;
			}
			if (!_location->getIndex().empty()) {
				_resource.append("/" + _location->getIndex());
				_resourceExists = true;
				_localPathResource.append(_resource);
			}
			else if (_location->getAutoindex())
				_autoIndex = true;
			else {
				_location = NULL; //para devolver 404
				return ;
			}
		}
		if(!checkAllowMethod())
			return ;
		if (locationHasRedirection(_location)) {
			_redirect = _location->getRedirect();
			_redirect.append(_resource);
			return ;
		}
		parseResource();
		_localPathResource = _localPathResource.substr(0, _localPathResource.find(_resource) + _resource.size());
		_resourceExists = checkFileExists(_localPathResource);

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
}

bool HttpRequest::appendPath(std::string &tmpPath, std::string const &uri)
{
  if (tmpPath.size() == uri.size())
    return false;
  if (tmpPath.empty()) {
    tmpPath.append("/");
    return true;
  }
// /images/algomas
  size_t end = uri.find('/', tmpPath.size() + 1);
  if (end == std::string::npos)
    end = uri.size();
  else
    end -= tmpPath.size();
  tmpPath.append(uri, tmpPath.size(), end);

  return true;
}

Location*	HttpRequest::findLocation(Server* Server)
{
  Logger::log(str("Looking for Location: ") + _uri, INFO);
  std::vector<Location*> locations = Server->getLocations();

  std::string locationPath;
  Location* location = NULL;
  for (std::size_t i = 0 ; i < locations.size(); i++) {
    locationPath = locations[i]->getUrlPath();
    Logger::log(str("Comparando Location: ") + _uri + " con: " + locationPath);

    bool found = false;
    std::string tmpPath;
    while (appendPath(tmpPath, _uri)) {
      if (locationPath == tmpPath) {
        found = true;
        break;
      }
    }
    if ((found && location == NULL) || (found && locationPath.size() > location->getUrlPath().size()))
      location = locations[i];
  }

  if (location != NULL)
    Logger::log("Location Encontrada: " + location->getUrlPath(), USER);
  else
    Logger::log("No se encontro location para este recurso: " + _uri, USER);
  return location;
}

bool	HttpRequest::checkAllowMethod()
{
	if (!_location) {
		Logger::log("Comprobando metodo en una location NULL");
		return false;
	}
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

bool HttpRequest::checkFileExists(str fullResource) {
    struct stat path_stat;
    if (stat(fullResource.c_str(), &path_stat) == 0)
        return S_ISREG(path_stat.st_mode);
    return false;
}

bool	HttpRequest::locationHasRedirection(const Location *loc) {
	if (!loc) {
		Logger::log("Location es null, no es posible comprobar redirect", ERROR);
		return false;
	}
	if (loc->getRedirect().empty())
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
Location*	HttpRequest::getLocation() const { return _location; }
str			HttpRequest::getReceivedMethod() const { return _receivedMethod; }
str			HttpRequest::getResource() const { return _resource; }
str			HttpRequest::getLocalPathResource() const { return _localPathResource; }
str			HttpRequest::getLocationUri() const { return _locationUri; }
str			HttpRequest::getQueryString() const { return _queryString; }
str			HttpRequest::getPathInfo() const { return _pathInfo; }
str			HttpRequest::getRedirect() const { return _redirect; }

//Setters
void		HttpRequest::setType(RequestType type) { _type = type; }
void		HttpRequest::setBadRequest(bool badRequest) { _badRequest = badRequest; }
void		HttpRequest::setResorceExist(bool resorceExist) { _resourceExists = resorceExist; }
void		HttpRequest::setValidMethod(bool validMethod) { _validMethod = validMethod; }
void		HttpRequest::setIsCgi(bool isCgi) { _isCgi = isCgi; }
void 		HttpRequest::setLocation(Location *location) { _location = location; }

HttpRequest::badHeaderException::badHeaderException(const str &msg) : _msg(msg) {}
const char	*HttpRequest::badHeaderException::what() const throw() { return _msg.c_str(); }
