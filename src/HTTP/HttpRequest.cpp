/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/31 13:31:03 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>

#define DEFAULT_ERROR "Invalid request line: expected '<METHOD> <PATH> <VERSION>' format"

const str HttpRequest::saveHeader(const str &request) {
	size_t pos = 0;
	size_t end;
	bool headerClosed = false;
	while ((end = request.find("\r\n", pos)) != str::npos) {
		str line = request.substr(pos, end - pos);
		pos = end + 2;
		if (line.empty()) {
			headerClosed = true;
			break;
		}

		std::size_t separador = line.find(":");
		if (separador == str::npos)
			throw badHeaderException("Header has not : separator");

		str key = line.substr(0, separador);
		str value = line.substr(separador + 2);
		_header[key] = value;
	}
	if (!headerClosed) // No hay \r\n\r\n
		throw badHeaderException("Header not terminated properly: missing \\r\\n\\r\\n");
	return request.substr(pos); // si no hay body, devuelve una linea vacia, '\0' o ("")
}

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
