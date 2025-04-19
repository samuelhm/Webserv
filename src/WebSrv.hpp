/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebSrv.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:23:53 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/14 12:06:14 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils/Logger.hpp"
#include <string>
#include <map>
#include <vector>
#define LIMIT_HEADER_SIZE 1024
#define NUMBER_OF_WORKERS 0

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

