/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebSrv.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:23:53 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/20 19:19:17 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <map>
#include <vector>
#define LIMIT_HEADER_SIZE 1024
#define NUMBER_OF_WORKERS 0
#define TIMEOUT_CGI 3

typedef std::string str;
typedef std::vector<str> strVec;
typedef strVec::iterator strVecIt;
typedef std::map<str, str> strMap;

enum	RequestType
{
	GET,
	POST,
	DELETE,
	OPTIONS,
	PUT,
	HEAD,
	TRACE,
	CONNECT
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

enum EventType
{
	NEWCONNECTION,
	RECIEVEREQUEST,
	SENDRESPONSE
};
