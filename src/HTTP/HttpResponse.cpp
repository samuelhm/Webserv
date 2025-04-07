/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/07 17:03:13 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include "HttpResponse.hpp"
#include "../Utils/Utils.hpp"

HttpResponse::HttpResponse(const HttpRequest &request, Server* server) : AHttp() {
	if (_statusStr.empty())
		fillStatusStr();
	if (request.getBadRequest()) {
		setErrorCode(400, server);
    return ;
  }
  if (request.getType() == GET)
    setResource(request, server);
}

void HttpResponse::setResource(const HttpRequest &request, Server* server)
{
  _status = 200;
  _line0.append("HTTP/1.1");
  _line0.append(_statusStr[_status]);
  str filepath;
  filepath.append(server->getRoot()); //NO ESTAMOS COMPROBANDO LOCATION ESTAMOS EN GET ROOT
  filepath.append(request.get_path());
  if (filepath[filepath.length() - 1] == '/')
    filepath.append(server->getLocations().at(0)->getIndex());
  _body.append(Utils::fileToStr(filepath));
  _body.append("\r\n");
  _header["Content-Type"] = "text/html\r\n";
  _header["Content-Length"] = Utils::intToStr(_body.length());
}

HttpResponse::HttpResponse(const HttpResponse &other) : AHttp(other) {
  *this = other;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setErrorCode(int ErrorCode, Server* server)
{
	_status = ErrorCode;
	_line0.append("HTTP/1.1");
  _line0.append(_statusStr[ErrorCode]);
  _body.append(server->getErrorPage(ErrorCode));
	_body.append("\r\n"); // FIN DE CABECERA
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
	// _header["Content-Length"].append("\r\n");
}

void HttpResponse::fillStatusStr() {
  _statusStr[100] = " 100 Continue\r\n";
  _statusStr[101] = " 101 Switching Protocols\r\n";
  _statusStr[200] = " 200 OK\r\n";
  _statusStr[201] = " 201 Created\r\n";
  _statusStr[202] = " 202 Accepted\r\n";
  _statusStr[203] = " 203 Non-Authoritative Information\r\n";
  _statusStr[204] = " 204 No Content\r\n";
  _statusStr[205] = " 205 Reset Content\r\n";
  _statusStr[206] = " 206 Partial Content\r\n";
  _statusStr[300] = " 300 Multiple Choices\r\n";
  _statusStr[301] = " 301 Moved Permanently\r\n";
  _statusStr[302] = " 302 Found\r\n";
  _statusStr[303] = " 303 See Other\r\n";
  _statusStr[304] = " 304 Not Modified\r\n";
  _statusStr[305] = " 305 Use Proxy\r\n";
  _statusStr[307] = " 307 Temporary Redirect\r\n";
  _statusStr[308] = " 308 Permanent Redirect\r\n";
  _statusStr[400] = " 400 Bad Request\r\n";
  _statusStr[401] = " 401 Unauthorized\r\n";
  _statusStr[402] = " 402 Payment Required\r\n";
  _statusStr[403] = " 403 Forbidden\r\n";
  _statusStr[404] = " 404 Not Found\r\n";
  _statusStr[405] = " 405 Method Not Allowed\r\n";
  _statusStr[406] = " 406 Not Acceptable\r\n";
  _statusStr[407] = " 407 Proxy Authentication Required\r\n";
  _statusStr[408] = " 408 Request Timeout\r\n";
  _statusStr[409] = " 409 Conflict\r\n";
  _statusStr[410] = " 410 Gone\r\n";
  _statusStr[411] = " 411 Length Required\r\n";
  _statusStr[412] = " 412 Precondition Failed\r\n";
  _statusStr[413] = " 413 Content Too Large\r\n";
  _statusStr[414] = " 414 URI Too Long\r\n";
  _statusStr[415] = " 415 Unsupported Media Type\r\n";
  _statusStr[416] = " 416 Range Not Satisfiable\r\n";
  _statusStr[417] = " 417 Expectation Failed\r\n";
  _statusStr[421] = " 421 Misdirected Request\r\n";
  _statusStr[422] = " 422 Unprocessable Content\r\n";
  _statusStr[426] = " 426 Upgrade Required\r\n";
  _statusStr[500] = " 500 Internal Server Error\r\n";
  _statusStr[501] = " 501 Not Implemented\r\n";
  _statusStr[502] = " 502 Bad Gateway\r\n";
  _statusStr[503] = " 503 Service Unavailable\r\n";
  _statusStr[504] = " 504 Gateway Timeout\r\n";
  _statusStr[505] = " 505 HTTP Version Not Supported\r\n";
}

std::map<int, std::string> HttpResponse::_statusStr;
