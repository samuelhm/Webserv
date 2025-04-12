/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebSrv.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:23:53 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/12 13:39:17 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Utils/Logger.hpp"
#include <string>
#include <map>
#include <vector>

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

