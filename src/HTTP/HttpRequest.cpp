/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/01 14:49:33 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HttpRequest.hpp"
#include <string>
#include <exception>
#include <iostream>

template <typename K, typename V>
void print_map(const std::map<K, V>& m) {
    // Use an iterator to loop through the map
    typename std::map<K, V>::const_iterator it;
    
    // Iterate through the map and print each key-value pair
    for (it = m.begin(); it != m.end(); ++it) {
        std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
    }
}

std::string ft_substr(const std::string& s, unsigned int start, size_t len)
{
    // If start is beyond the string length, return empty string
    if (start >= s.length())
        return "";

    // Calculate the effective length - don't read past end of string
    size_t effective_len = std::min(len, s.length() - start);
    
    // Return the substring using std::string's substr method
    return s.substr(start, effective_len);
}

const str HttpRequest::saveHeader(const str &request) {
	if (request.find(": ") == str::npos) {
		return request;
	}
	int start = request.find(": ");
	int end = request.find("\r\n");
	_header[ft_substr(request, 0, start)] = ft_substr(request, start + 2, end);
	saveHeader(ft_substr(request, start + 2, end - start - 2));
	// std::cout << request.substr(0, start) << "---" << request.substr(start + end + 2);
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
		print_map(_header);
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
