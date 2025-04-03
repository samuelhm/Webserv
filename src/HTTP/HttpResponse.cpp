/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                    :+:      :+:    :+: */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 16:16:06 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include <cstdlib>
#include "../Utils/Utils.hpp"

HttpResponse::HttpResponse(const HttpRequest &request, Server* server) : AHttp() {
	if (_statusStr.empty())
		fillStatusStr();
	if (request.getBadRequest())
		setErrorCode(400, server);
}

HttpResponse::HttpResponse(const HttpResponse &other) : AHttp(other) {
  *this = other;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setErrorCode(int ErrorCode, Server* server)
{
	_status = ErrorCode;
	_line0.append("HTTP/1.1 ");
	_line0.append(Utils::intToStr(ErrorCode));
	_line0.append(_statusStr[ErrorCode]);
	_header["Content-Type"] = "text/html; charset=UTF-8\r\n";
	_header["Content-Length"] = Utils::intToStr(_header["Content-Type"].length());
	_header["Content-Length"].append("\r\n");
	_body.append("\r\n"); // FIN DE CABECERA

  // std::cout << server->getErrorPages()[ErrorCode];

  if (server == NULL)
    std::cout  << "hola\n";
  Utils::print_map(server->getErrorPages());
	// _body.append(server->getErrorPages()[ErrorCode]);
}

void HttpResponse::fillStatusStr() {
  _statusStr[100] = " Continue\r\n";
  _statusStr[101] = " Switching Protocols\r\n";
  _statusStr[200] = " OK\r\n";
  _statusStr[201] = " Created\r\n";
  _statusStr[202] = " Accepted\r\n";
  _statusStr[203] = " Non-Authoritative Information\r\n";
  _statusStr[204] = " No Content\r\n";
  _statusStr[205] = " Reset Content\r\n";
  _statusStr[206] = " Partial Content\r\n";
  _statusStr[300] = " Multiple Choices\r\n";
  _statusStr[301] = " Moved Permanently\r\n";
  _statusStr[302] = " Found\r\n";
  _statusStr[303] = " See Other\r\n";
  _statusStr[304] = " Not Modified\r\n";
  _statusStr[305] = " Use Proxy\r\n";
  _statusStr[307] = " Temporary Redirect\r\n";
  _statusStr[308] = " Permanent Redirect\r\n";
  _statusStr[400] = " Bad Request\r\n";
  _statusStr[401] = " Unauthorized\r\n";
  _statusStr[402] = " Payment Required\r\n";
  _statusStr[403] = " Forbidden\r\n";
  _statusStr[404] = " Not Found\r\n";
  _statusStr[405] = " Method Not Allowed\r\n";
  _statusStr[406] = " Not Acceptable\r\n";
  _statusStr[407] = " Proxy Authentication Required\r\n";
  _statusStr[408] = " Request Timeout\r\n";
  _statusStr[409] = " Conflict\r\n";
  _statusStr[410] = " Gone\r\n";
  _statusStr[411] = " Length Required\r\n";
  _statusStr[412] = " Precondition Failed\r\n";
  _statusStr[413] = " Content Too Large\r\n";
  _statusStr[414] = " URI Too Long\r\n";
  _statusStr[415] = " Unsupported Media Type\r\n";
  _statusStr[416] = " Range Not Satisfiable\r\n";
  _statusStr[417] = " Expectation Failed\r\n";
  _statusStr[421] = " Misdirected Request\r\n";
  _statusStr[422] = " Unprocessable Content\r\n";
  _statusStr[426] = " Upgrade Required\r\n";
  _statusStr[500] = " Internal Server Error\r\n";
  _statusStr[501] = " Not Implemented\r\n";
  _statusStr[502] = " Bad Gateway\r\n";
  _statusStr[503] = " Service Unavailable\r\n";
  _statusStr[504] = " Gateway Timeout\r\n";
  _statusStr[505] = " HTTP Version Not Supported\r\n";
}

std::map<int, std::string> HttpResponse::_statusStr;
