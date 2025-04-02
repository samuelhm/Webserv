#ifndef WEBSRV_HPP
#define WEBSRV_HPP

#include <string>

typedef std::string str;

enum	RequestType
{
	GET,
	POST,
	DELETE,
	OPTIONS
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
