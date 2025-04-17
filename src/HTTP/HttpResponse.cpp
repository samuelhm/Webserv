/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/17 23:27:27 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "../Utils/Utils.hpp"
#include <cstdlib>

HttpResponse::HttpResponse(int errorCode, Server *server) : AHttp()
{
	setErrorCode(errorCode, server);
}

void  HttpResponse::staticFileGet(const HttpRequest &request, Server* server)
{
  str filepath = request.getLocalPathResource();
	try {
		_body = Utils::fileToStr(filepath);
		_status = 200;
		_line0.append("HTTP/1.1 ");
		_line0.append(Utils::intToStr(_status) + " ");
		_line0.append(Utils::_statusStr[_status] + "\r\n");
		_header["Content-Type"] = Utils::getMimeType(filepath) + "\r\n";
		_header["Content-Length"] = Utils::intToStr(_body.length());
	}
	catch (const std::exception &e) {
		Logger::log(str("Error abriendo archivo en GET: ") + e.what(), WARNING);
		setErrorCode(404, server);
	}
}

void HttpResponse::staticFilePost(const HttpRequest &request, Server* server)
{
	Location* loc = request.getLocation();
	if (!loc || !loc->getUploadEnable()) {
		Logger::log("POST rechazado: upload no habilitado en esta location", WARNING);
		setErrorCode(403, server);
		return;
	}
	const str& body = request.getBody();
	if (body.empty()) {
		Logger::log("POST rechazado: cuerpo vacío", WARNING);
		setErrorCode(400, server);
		return;
	}
	if (body.size() > static_cast<size_t>(server->getBodySize())) {
		Logger::log("POST rechazado: body excede client_max_body_size", WARNING);
		setErrorCode(413, server);
		return;
	}

	str fullPath = server->getRoot() + loc->getRoot() + loc->getUploadPath() + request.getResource();
	std::ofstream out(fullPath.c_str(), std::ios::binary);
	if (!out.is_open()) {
		Logger::log(str("POST error: no se pudo crear el archivo en disco: ") + fullPath, ERROR);
		setErrorCode(500, server);
		return;
	}
	out << body; //IMPORTANT Sobreescribir por defecto? o detectar que existe y mandar error?
	out.close();
	_status = 201;
	_line0 = "HTTP/1.1 201 Created\r\n";
	_body = "<html><body><h1>File upload Success: " + request.getResource() + "</h1></body></html>";
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}

void HttpResponse::staticFilePut(const HttpRequest &request, Server* server)
{
	Location* loc = request.getLocation();
	const str& body = request.getBody();
	if (body.empty()) {
		Logger::log("PUT rechazado: cuerpo vacío", WARNING);
		setErrorCode(400, server);
		return;
	}
	if (body.size() > static_cast<size_t>(server->getBodySize())) {
		Logger::log("PUT rechazado: body excede client_max_body_size", WARNING);
		setErrorCode(413, server);
		return;
	}
	str fullPath = server->getRoot() + loc->getRoot() + request.getResource();
  std::ifstream check(fullPath.c_str(), std::ios::binary);
  bool exist = check.is_open();
	_status = exist ? 200 : 201;

	std::ofstream out(fullPath.c_str(), std::ios::binary | std::ios::trunc);
	if (!out.is_open()) {
		Logger::log("PUT error: no se pudo abrir el archivo para escritura: " + fullPath, ERROR);
		setErrorCode(500, server);
		return;
	}
	out << body;
	out.close();
	_line0 = "HTTP/1.1 " + Utils::intToStr(_status) + " " + Utils::_statusStr[_status] + "\r\n";
	_body = "<html><body><h1>File " + (exist ? str("actualizado") : str("Created")) + " correctly.</h1></body></html>";
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}

void HttpResponse::staticFileOptions(const HttpRequest &request, Server* server)
{
	Location* loc = request.getLocation();
	if (!loc) {
		Logger::log("OPTIONS rechazado: no se encontró Location", WARNING);
		setErrorCode(404, server);
		return;
	}
	const std::vector<RequestType>& methods = loc->getMethods();
	str allowed;
	for (size_t i = 0; i < methods.size(); ++i) {
		if (i > 0)
			allowed += ", ";
		allowed += Utils::requestTypeToStr(methods[i]);
	}
	_status = 204;
	_line0 = "HTTP/1.1 204 No Content\r\n";
	_header["Allow"] = allowed + "\r\n";
	_header["Content-Length"] = "0";
	_header["Content-Type"] = "text/plain\r\n";
	_body.clear();
}

void HttpResponse::staticFileDelete(const HttpRequest &request, Server* server)
{
	Location* loc = request.getLocation();
	str path = server->getRoot() + loc->getRoot() + request.getResource();

	std::ifstream check(path.c_str());
	if (!check.is_open()) {
		Logger::log("DELETE: recurso no encontrado: " + path, WARNING);
		setErrorCode(404, server);
		return;
	}
	check.close();
	if (std::remove(path.c_str()) != 0) {
		Logger::log("DELETE: fallo al eliminar recurso con std::remove(): " + path, ERROR);
		setErrorCode(500, server);
		return;
	}
	_status = 200;
	_line0 = "HTTP/1.1 200 OK\r\n";
	_body = "<html><body><h1>Resource has been deleted.</h1></body></html>";
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}

void  HttpResponse::staticFileExec(const HttpRequest &request, Server *server)
{
  switch (request.getType())
  {
    case GET: staticFileGet(request, server); break;
    case POST: staticFilePost(request, server); break;
    case PUT: staticFilePut(request, server); break;
    case OPTIONS: staticFileOptions(request, server); break;
    case DELETE: staticFileDelete(request, server); break;

    default:
      Logger::log("Response construcor cannot found valid method", ERROR);
    break;
  }
}

HttpResponse::HttpResponse(const HttpRequest &request, Server* server) : AHttp() {
	if (request.getIsCgi())
    cgiExec(request);
  else
    staticFileExec(request, server);
}

HttpResponse::HttpResponse(const HttpResponse &other) : AHttp(other) {
  *this = other;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setErrorCode(int errorCode, Server* server)
{
	_line0.clear();
	_body.clear();
	_header.clear();

	_status = errorCode;

	std::map<int, str>::iterator it = Utils::_statusStr.find(errorCode);
	if (it == Utils::_statusStr.end()) {
		Logger::log("Código de estado no reconocido: " + Utils::intToStr(errorCode), WARNING);
		errorCode = 500;
		_status = 500;
		it = Utils::_statusStr.find(500);
	}
	_line0 = "HTTP/1.1 " + Utils::intToStr(errorCode) + " " + it->second + "\r\n";
	try {
		_body = server->getErrorPage(errorCode);
	} catch (...) {
		Logger::log("Fallo al obtener página de error personalizada. Usando default.", WARNING);
		_body = "<html><body><h1>" + Utils::intToStr(errorCode) + " " + it->second + "</h1></body></html>";
	}
	_header["Content-Type"] = "text/html\r\n";
	_header["Content-Length"] = Utils::intToStr(_body.length());
}
