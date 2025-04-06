/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:08 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/06 14:12:38 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../WebSrv.hpp"
#include "ParseConfig.hpp"
#include "Location.hpp"
#include "../HTTP/HttpResponse.hpp"

void setLocationParams(Location *location, std::map<str, str> const &options);
bool isValidPath(std::string const &path);
std::string getLocationPath(std::string const &locationString);


