
#include "Utils.hpp"
#include "../ConfigFile/Server.hpp"
#include "../ConfigFile/ParseConfig.hpp"
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>

namespace Utils {

	strVec split(const str &input, char delimiter) {
		strVec tokens;
		str token;
    if (input.empty())
      return tokens;
		for (str::size_type i = 0; i < input.length(); ++i) { //Usamos std::string::size_type porque lenght() y las demas devuelven este tipo, si usamos int el compilador da warnings.
			if (input[i] == delimiter) {
				tokens.push_back(token); //Cuandoo encuentra delimiter, pushea y limpia.
				token.clear();
			} else {
				token += input[i]; //No es delimiter, mete la letra en el str actual.
			}
		}
    tokens.push_back(token); //Aqui no ha encontrado mas delimiters mete el resto.
		return tokens;
	}

	strVec split(const str &input, const str &delimiter) {
		strVec tokens;
		str::size_type start = 0;
		str::size_type pos;

		while ((pos = input.find(delimiter, start)) != str::npos) {
			tokens.push_back(input.substr(start, pos - start));
			start = pos + delimiter.length();
		}
		tokens.push_back(input.substr(start));
		return tokens;
	}
	/* isspace() incluye:
	*	' ' (espacio)
	*	'\t' (tabulación)
	*	'\n' (salto de línea)
	*	'\r' (retorno de carro)
	*	'\v' (tab vertical)
	*	'\f' (form feed)
	*	Usa static_cast en c++98 para evitar errores con EOF https://en.cppreference.com/w/cpp/string/byte/isspace
	*/
	str trim(const str &s) {
		str::size_type start = 0;
		while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
			++start;
		str::size_type end = s.size();
		while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
			--end;
		return s.substr(start, end - start);
	}

	//File se cierra automaticamente al salir de su ambito con su destructor, no es necesario usar close();
  str fileToStr(const str &filePath) {
		Logger::log(str("Trying to open file: ") + filePath, INFO);
		std::ifstream file(filePath.c_str());
		if (!file.is_open())
			throw std::runtime_error("File not found: " + filePath);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	str intToStr(std::size_t num)
	{
		if (num == 0)
			return "0";
		str response;
		while (num != 0)
		{
			char digit = (num % 10) + '0';
			response.push_back(digit);
			num /= 10;
		}
		std::reverse(response.begin(), response.end());
		return response;
	}

	// str intToStr(int num)
	// {
	// 	std::ostringstream result;
	// 	result << num;
	// 	return result.str();
	// }
}

void Utils::fillStatusStr() {
	_statusStr[100] = "Continue";
	_statusStr[101] = "Switching Protocols";
	_statusStr[200] = "OK";
	_statusStr[201] = "Created";
	_statusStr[202] = "Accepted";
	_statusStr[203] = "Non-Authoritative Information";
	_statusStr[204] = "No Content";
	_statusStr[205] = "Reset Content";
	_statusStr[206] = "Partial Content";
	_statusStr[300] = "Multiple Choices";
	_statusStr[301] = "Moved Permanently";
	_statusStr[302] = "Found";
	_statusStr[303] = "See Other";
	_statusStr[304] = "Not Modified";
	_statusStr[305] = "Use Proxy";
	_statusStr[307] = "Temporary Redirect";
	_statusStr[308] = "Permanent Redirect";
	_statusStr[400] = "Bad Request";
	_statusStr[401] = "Unauthorized";
	_statusStr[402] = "Payment Required";
	_statusStr[403] = "Forbidden";
	_statusStr[404] = "Not Found";
	_statusStr[405] = "Method Not Allowed";
	_statusStr[406] = "Not Acceptable";
	_statusStr[407] = "Proxy Authentication Required";
	_statusStr[408] = "Request Timeout";
	_statusStr[409] = "Conflict";
	_statusStr[410] = "Gone";
	_statusStr[411] = "Length Required";
	_statusStr[412] = "Precondition Failed";
	_statusStr[413] = "Content Too Large";
	_statusStr[414] = "URI Too Long";
	_statusStr[415] = "Unsupported Media Type";
	_statusStr[416] = "Range Not Satisfiable";
	_statusStr[417] = "Expectation Failed";
	_statusStr[421] = "Misdirected Request";
	_statusStr[422] = "Unproce_statusStrable Content";
	_statusStr[426] = "Upgrade Required";
	_statusStr[500] = "Internal Server Error";
	_statusStr[501] = "Not Implemented";
	_statusStr[502] = "Bad Gateway";
	_statusStr[503] = "Service Unavailable";
	_statusStr[504] = "Gateway Timeout";
	_statusStr[505] = "HTTP Version Not Supported";
  }
  bool	Utils::setUpServers(std::vector<Server*>& servers)
  {
    for (size_t i = 0; i < servers.size(); ++i) {
		try {
			servers[i]->socketUp();
		} catch (const std::exception& e) {
			return false;
		}
	}
	return true;
  }

  std::map<int, str>	Utils::_statusStr;

void Utils::printRequest(HttpRequest &request)
{
	Logger::log("----- HttpRequest Info -----", WARNING);
	if (request.getBadRequest()) {
		Logger::log("Bad Request: true", WARNING);
		return;
	}
	Logger::log("Bad Request: false", WARNING);
	Logger::log(str("Request Type: ") + requestTypeToStr(request.getType()), WARNING);
	Logger::log(str("Received Method: ") + request.getReceivedMethod(), WARNING);
	Logger::log(str("Resource: ") + request.getResource(), WARNING);
	Logger::log(str("Resource Exists: ") + (request.getResourceExists() ? "true" : "false"), WARNING);
	Logger::log(str("Valid Method: ") + (request.getValidMethod() ? "true" : "false"), WARNING);
	Logger::log(str("Is CGI: ") + (request.getIsCgi() ? "true" : "false"), WARNING);
	Logger::log(str("Local Path Resource: ") + request.getLocalPathResource(), WARNING);
	Location* loc = request.getLocation();
	if (loc)
		Logger::log("Location pointer: (valida)", WARNING);
	else
		Logger::log("Location pointer: null", WARNING);
	Logger::log(str("Location URI: ") + request.getLocationUri(), WARNING);
	if (request.getIsCgi()) {
		Logger::log(str("Query String: ") + request.getQueryString(), WARNING);
		Logger::log(str("Path Info: ") + request.getPathInfo(), WARNING);
	}
	Logger::log(str("Redirect: ") + (request.getRedirect().empty() ? "false" : request.getRedirect()), WARNING);
	Logger::log("----------------------------", WARNING);
}

str Utils::requestTypeToStr(RequestType type) {
	switch (type) {
		case GET: return "GET"; break;
		case POST: return "POST"; break;
		case DELETE: return "DELETE"; break;
		case OPTIONS: return "OPTIONS"; break;
		case PUT: return "PUT"; break;
		default: return "UNKNOWN";
	}
}

HttpResponse &Utils::codeResponse(int errorCode, Server *server)
{
	static HttpResponse resp400(400, server);
	static HttpResponse resp403(403, server);
	static HttpResponse resp404(404, server);
	static HttpResponse resp405(405, server);
	static HttpResponse resp413(413, server);
	static HttpResponse resp414(414, server);
	static HttpResponse resp500(500, server);

	switch (errorCode)
	{
		case 400: return resp400;
		case 403: return resp403;
		case 404: return resp404;
		case 405: return resp405;
		case 413: return resp413;
		case 414: return resp414;
		case 500: return resp500;
		default:
			Logger::log("getStaticErrorResponse: código no soportado: " + intToStr(errorCode), WARNING);
			return resp500; // fallback
	}
}

str Utils::getMimeType(const str &filename)
{
	static std::map<str, str> mimeTypes;
	if (mimeTypes.empty()) {
		mimeTypes[".html"] = "text/html";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".json"] = "application/json";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".xml"] = "application/xml";
		mimeTypes[".zip"] = "application/zip";
		mimeTypes[".tar"] = "application/x-tar";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".webm"] = "video/webm";
		mimeTypes[".wasm"] = "application/wasm";
	}
	std::size_t dotPos = filename.find_last_of('.');
	if (dotPos != str::npos) {
		str ext = filename.substr(dotPos);
		if (mimeTypes.find(ext) != mimeTypes.end())
			return mimeTypes[ext];
	}
	return "application/octet-stream";
}

bool Utils::isDirectory(const std::string &path) {
	DIR *dir = opendir(path.c_str());
	if (dir) {
		closedir(dir);
		return true;
	}
	return false;
}

Location*	Utils::findLocation(Server* Server, const str &uri)
{
  Logger::log(str("Looking for Location: ") + uri, INFO);
  std::vector<Location*> locations = Server->getLocations();

  std::string locationPath;
  Location* location = NULL;
  for (std::size_t i = 0 ; i < locations.size(); i++) {
    locationPath = locations[i]->getUrlPath();
    bool found = false;
    std::string tmpPath;
    while (appendPath(tmpPath, uri)) {
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
    Logger::log("No se encontro location para este recurso: " + uri, USER);
  return location;
}

bool Utils::appendPath(std::string &tmpPath, std::string const &uri)
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

bool Utils::atoi(const char *str, int &out)
{
	errno = 0;
	char *end;
	long val = std::strtol(str, &end, 10);

	if (end == str) {
		Logger::log("Error: no se encontró ningún número válido", WARNING);
		return false;
	}
	if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
		Logger::log("Error: no se encontró ningún número válido", WARNING);
		return false;
	}
	out = static_cast<int>(val);
	return true;
}
