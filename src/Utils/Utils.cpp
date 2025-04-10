
#include <algorithm>
#include "Utils.hpp"
#include "../ConfigFile/Server.hpp"
#include "../ConfigFile/ParseConfig.hpp"
namespace Utils {

	std::vector<str> split(const str &input, char delimiter) {
		std::vector<str> tokens;
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

	std::vector<str> split(const str &input, const str &delimiter) {
		std::vector<str> tokens;
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
