/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/02 08:48:09 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>

const str HttpRequest::saveHeader(const str &request) {
    int end = request.find("\r\n");
    if (end == str::npos)
        return request;
    str line = request.substr(0, end);
    int separator = line.find(": ");
    if (separator != str::npos) {
        str key = line.substr(0, separator);
        str value = line.substr(separator + 2);
        _header[key] = value;
    }
    if (end + 2 < (int)request.length()) {
        return saveHeader(request.substr(end + 2));
    }
    return request;
}

#define DEFAULT_ERROR "Invalid request line: expected '<METHOD> <PATH> <VERSION>' format"

void HttpRequest::checkHeaderMRP(const str &line) {
	std::size_t end = line.find(" ", 0);
	if (end == str::npos)
		throw badHeaderException(DEFAULT_ERROR);

	str	method = line.substr(0, end);
	if (method == "GET") _type = GET;
	else if (method == "POST") _type = POST;
	else if (method == "OPTIONS") _type = OPTIONS;
	else if (method == "DELETE") _type = DELETE;
	else throw badHeaderException("Method not allowed or wrong");

	if (line.at(++end) != '/') // IMPORTANT probar si hay mas espacios
		throw badHeaderException("Invalid Resource Format");
	size_t path_end = line.find(" ", end);
	if (path_end == str::npos)
		throw badHeaderException(DEFAULT_ERROR);
	_path = line.substr(end, path_end - end);

	if (line.substr(path_end + 1) != "HTTP/1.1\r\n") { //aqui no tinene \r\n porque ya lo quitamos en el constructor
		throw badHeaderException("Protocol version not supported");
	}
}

HttpRequest::HttpRequest(str request) : AHttp(request), _badRequest(false) {
	int end = request.find("\r\n");
	if (end == str::npos)
		throw badHeaderException(DEFAULT_ERROR);
	const str line = request.substr(0, end + 2);
	try {
		checkHeaderMRP(line);
		_body = saveHeader(request.substr(end));
	} catch(const badHeaderException &e) {
		std::cout << e.what() << std::endl;
		_badRequest = true;
		return ;
	} catch(...) {
		_badRequest = true;
		std::cout << "error: extra" << std::endl;
	}
}

HttpRequest::~HttpRequest() {}

//Getters
RequestType	HttpRequest::getType() const { return _type; }
bool		HttpRequest::getBadRequest() const { return _badRequest; }

HttpRequest::badHeaderException::badHeaderException(const std::string &msg) : _msg(msg) {}
const char *HttpRequest::badHeaderException::what() const throw() { return _msg.c_str(); }
