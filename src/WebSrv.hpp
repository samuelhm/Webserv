#ifndef WEBSRV_HPP
#define WEBSRV_HPP


#include <string>
#include "Utils/Logger.hpp"

typedef std::string str;

enum	RequestType
{
	GET,
	POST,
	DELETE,
	OPTIONS,
	PUT
};

enum OptionType
{
	SERVERNAME,
	LISTEN,
	ISDEFAULT,
	ROOT,
	BODYSIZE,
	ERRORPAGE,
	LOCATION
};

#endif
