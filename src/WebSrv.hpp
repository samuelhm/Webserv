#ifndef WEBSRV_HPP
#define WEBSRV_HPP


#include <string>
#include <map>
#include <vector>
#include "Utils/Logger.hpp"

typedef std::string str;
typedef std::vector<str> strVec;
typedef std::map<str, str> strMap;

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
