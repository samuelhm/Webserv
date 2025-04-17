/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/17 15:12:59 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "../Utils/Utils.hpp"
#include <cstdlib>

HttpResponse::HttpResponse::HttpResponse(int errorCode) : AHttp()
{
	_status = errorCode;
	_line0.append("HTTP/1.1 ");
	_line0.append(Utils::intToStr(errorCode));
	_line0.append(" ");
	_line0.append(Utils::_statusStr[errorCode]);
	_line0.append("\r\n");
	_body = "<html><head><title>";
	_body += Utils::intToStr(errorCode) + " " + Utils::_statusStr[errorCode];
	_body += "</title></head><body><h1>";
	_body += Utils::intToStr(errorCode) + " " + Utils::_statusStr[errorCode];
	_body += "</h1></body></html>\r\n";
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}


HttpResponse::HttpResponse(const HttpRequest &request, Server* server) : AHttp() {
	if (!request.getIsCgi())
    setResource(request, server);
  else
    cgiExec(request);
}

void HttpResponse::setResource(const HttpRequest &request, Server* server)
{
  _status = 200;
  _line0.append("HTTP/1.1 ");
  _line0.append(Utils::intToStr(_status) + " ");
  _line0.append(Utils::_statusStr[_status]);
  _line0.append("\r\n");
  str filepath;
  filepath.append(server->getRoot()); // NO ESTAMOS COMPROBANDO LOCATION ESTAMOS EN GET ROOT
  if (!request.getLocation()->getRoot().empty())
    filepath.append(request.getLocation()->getRoot());
  if (filepath[filepath.length() - 1] == '/')
    filepath.append(server->getLocations().at(0)->getIndex());
  try { _body.append(Utils::fileToStr(filepath)); }
  catch (std::runtime_error &e) {
    Logger::log("Not found resource", USER);
    _body.append(server->getErrorPage(404));
  }
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
	_line0.append("HTTP/1.1 ");
  _line0.append(Utils::intToStr(ErrorCode) + " ");
  if (Utils::_statusStr[400].empty())
    Logger::log("Error code is empty", INFO);
  _line0.append(Utils::_statusStr[ErrorCode] + "\r\n");
  _body.append(server->getErrorPage(ErrorCode));
	_body.append("\r\n"); // FIN DE CABECERA
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}
