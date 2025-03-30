/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/30 17:22:18 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>

const str HttpRequest::saveHeader(const str &request) {
	if (request.find(": ") == str::npos) {
		return request;
	}
	int start = request.find(": ");
	int end = request.find("\r\n");
	_header[request.substr(0, start)] = request.substr(start, end);
	saveHeader(request.substr(start + end));
	return request;
}

void HttpRequest::checkHeaderMRP(const str &line) {
	std::size_t end = line.find(" ", 0);
	if (end == str::npos)
		throw badHeaderException();

	str	method = line.substr(0, end);
	if (method == "GET")
		_type = GET;
	else if (method == "POST")
		_type = POST;
	else if (method == "OPTIONS")
		_type = OPTIONS;
	else if (method == "DELETE")
		_type = DELETE;
	else
		throw new badHeaderException;

	if (line.at(end + 1) != '/') // IMPORTANT probar si hay mas espacios
		throw badHeaderException();
	end = line.find(" ", end + 1);
	if (end == str::npos)
		throw badHeaderException();
	if (line.substr(end + 1).compare("HTTP/1.1\r\n") != 0) {
		throw badHeaderException();
	}
}
HttpRequest::HttpRequest(str request) : AHttp(request), _badRequest(false) {
	int start = 0, end;
	end = request.find("\r\n", start);

	if (end == str::npos)
	{
		std::cout << str::npos << std::endl;
		throw badHeaderException();
	}
	end += 2;
	const str line = request.substr(start, end);
	try {
		checkHeaderMRP(line);
		saveHeader(request.substr(end));
	} catch(const badHeaderException &e) {
		std::cout << e.what() << std::endl;
		_badRequest = true;
		return ;
	} catch(...) {
		std::cout << "error: extra" << std::endl;
	}
}

HttpRequest::HttpRequest(const HttpRequest &other) : AHttp(other) { *this = other; }

HttpRequest::~HttpRequest() {}


const char *HttpRequest::badHeaderException::what() const throw () { return "EL Header esta mal"; }
