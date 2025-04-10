/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/10 12:17:04 by erigonza         ###   ########.fr       */
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
	size_t slash = _path.find_last_of('/');
	size_t pos_var = _path.find('?');
	if (pos_var != str::npos) {
		_resource = _path.substr(slash , pos_var - slash);
		_varCgi = _path.substr(pos_var + 1);
	}
	else
		_resource = _path.substr(slash);

	if (line.substr(path_end + 1) != "HTTP/1.1\r\n") //aqui no tinene \r\n porque ya lo quitamos en el constructor
		throw badHeaderException("Bad Protocol version");
}

bool HttpRequest::checkResource(Server const &server) {
  _resorceExist = false;
  if (!_location)
    return false;
  str root = _location->getRoot();
  if (root.empty())
    root = server.getRoot();
  std::ifstream resource((root + _resource).c_str());
  _resorceExist = resource.good();
  return resource;
}

Location*	HttpRequest::getLocation(Server* Server) {
	Logger::log(str("Looking for Location: ") + _path, INFO);
	std::vector<Location*> locations = Server->getLocations();
	for (int i = 0 ; i < locations.size(); i++) {
		Logger::log(str("Comparando Location: ") + _path + "con: " + locations[i]->getUrlPath());
		if (_path == locations[i]->getUrlPath()) {
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

void	HttpRequest::checkIsValidCgi() {
	if (!_location->getCgiEnable())
		return ;
	std::ifstream isValidCgi((_location->getCgiPath()).c_str());
	_isValidCgi = isValidCgi.good();
}

void	HttpRequest::checkIsCgi(Server *server)
{
	str ext;
	if (_resource.find('.') != str::npos)
		ext = _resource.substr(_resource.find('.') - 1);
	else
		return ;
	for (size_t i = 0; i < _location->getCgiExtension().size(); i++) {
		if (_location->getCgiExtension()[i] == ext) {
			_isCgi = true;
			break ;
		}
	}
	if (_isCgi == true)
		checkIsValidCgi();
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
		_location = getLocation(server);
		if(!checkResource(*server))
			return ;
		if(!checkAllowMethod())
			return ;
		checkIsCgi(server);
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

HttpRequest::badHeaderException::badHeaderException(const std::string &msg) : _msg(msg) {}
const char *HttpRequest::badHeaderException::what() const throw() { return _msg.c_str(); }
