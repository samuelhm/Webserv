#include "ParseConfig.hpp"


std::vector<Server*>	parseConfigFile(const str &filepath) {
	str content = Utils::fileToStr(filepath); // si falla lanza una excepcion, creo que es mejor recogerla en el main
	std::vector<str> serverStrings = Utils::split(content, '}');
	if (serverStrings.empty())
		throw ConfigFileException("No Server found inside config file");
	std::vector<Server*> result;
	for (std::vector<str>::iterator it = serverStrings.begin(); it != serverStrings.end(); ++it) {
		try {
			result.push_back(GetServer(*it));
		} catch (ConfigFileException &e) {
			std::cout << "Error parsing server: " << e.what() << std::endl; // ¿Return aqui y limpiamos memoria, o aceptamos el resto de servers validos?
		}
	}
	return result;
}

bool	isValidOption(const str &line, OptionType &type)
{
	bool result = true;
	if (line.find("server_name:") == 0)//== 0 para que la cadena justo empiece en el principio y no contenga caracteres invalidos. viene trimeada
		type = SERVERNAME;
	else if (line.find("listen:") == 0)
		type = LISTEN;
	else if (line.find("is_default:") == 0)
		type = ISDEFAULT;
	else if (line.find("root:") == 0)
		type = ROOT;
	else if (line.find("client_max_body_size:") == 0)
		type = BODYSIZE;
	else if (line.find("error_") == 0 &&
			line.size() >= 10 &&
			std::isdigit(line[6]) &&
			std::isdigit(line[7]) &&
			std::isdigit(line[8]) &&
			line[9] == ':')
	{
		type = ERRORPAGE;
	}
	else if (line.find("location:") == 0)
		type = LOCATION;
	else
		result = false;
	return result;
}

void	insertOption(const str &value, int type, Server* server)
{
	if (value.empty()) {
		throw EmptyValueException();
	}
	switch (type)
	{
	case SERVERNAME:
		server->setServerName(value);
		break;
	case ISDEFAULT:
		bool tmp = value == "yes" ? true : false;
		server->setIsdefault(tmp);
		break;
	case ROOT:
		server->setRoot(value);
		break;
	case BODYSIZE:
		server->setBodySize(std::atoi(value.c_str()));
		break;
	case LISTEN:
	{
		size_t sep = value.find(":");
		if (sep == std::string::npos)
			throw ConfigFileException("LISTEN must be in format hostname:port" + value);
		str hostname = Utils::trim(value.substr(0, sep));
		str port = Utils::trim(value.substr(sep + 1));
		if (hostname.empty() || port.empty())
			throw ConfigFileException("LISTEN has empty host or port: " + value);
		server->setHostName(hostname);
		server->setPort(port);
		break;
	}
	default:
		throw std::exception();
		break;
	}
}

Server*	GetServer(const str &serverString)
{
	std::istringstream ss(serverString);
	str line;
	std::getline(ss, line);
	line = Utils::trim(line);
	if ( line != "{")
		throw ConfigFileException("Server must be between brackets {}");
	Server* server = new Server();
	while (std::getline(ss, line)) {
		line = Utils::trim(line);
		OptionType type;
		if (line.empty())
			continue ;
		if (!isValidOption(line, type)) {
			throw ConfigFileException("Wrong option: " + line);
		}
		if (type != LOCATION) //Todos menos location, que habrá que montar una string con varias lineas
		{
			try {
				if ( type != ERRORPAGE) {
					str value = Utils::trim(line.substr(line.find(":") + 1));
					insertOption(value, type, server);
				}
				else { //Solo ERRORPAGE
					size_t code_start = 6; // pos 6 es el primer numero.
					str code_str = line.substr(code_start, 3); //guardamos los 3 numeros del error.
					int code = std::atoi(code_str.c_str()); // Lo pasamos a integer para el map.
					str path = Utils::trim(line.substr(line.find(":") + 1)); //sacamos el path, desde el : +1.
					if (code < 100 || code > 599)
						throw ConfigFileException("Invalid error code: " + code_str);
					server->getErrorPages()[code] = path;
				}
			} catch (EmptyValueException &e) {
				std::cout << e.what() << "ignoring option." << std::endl;
				continue ;
			} catch (...) { throw ConfigFileException("Unknown Error (insertOption())"); }
		}
		else {
			str locationBlock = line + "\n";
			bool foundClosingBracket = false;
			while (std::getline(ss, line)) {
				line = Utils::trim(line);
				locationBlock += line + "\n";
				if (line == "]") {
					foundClosingBracket = true;
					break;
				}
			}
			if (!foundClosingBracket)
				throw ConfigFileException("Location block not closed with ']'");
			Location location = getLocation(locationBlock);
			server->getLocations().push_back(location);
		}

	}
	return (server);
}

const char* EmptyValueException::what() const throw() {	return "You cannot Assign empty value"; }
