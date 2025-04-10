/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:08 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/06 20:35:02 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../WebSrv.hpp"
#include "ParseConfig.hpp"
#include "Location.hpp"
#include "../HTTP/HttpResponse.hpp"

void		setLocationParams(Location *location, strMap const &options);
bool 		isValidPath(std::string const &path);
std::string getLocationPath(std::string const &locationString);
Location	*getLocation(const str &locationString, const str &serverName);
RequestType strToRequest(const str &method);

class BadOptionLocationException : public std::exception
{
	public:
    virtual const char* what() const throw();
};
class BadSyntaxLocationBlockException : public std::exception {
	private:
		std::string _msg;
	public:
    virtual ~BadSyntaxLocationBlockException(void) throw() {}
		BadSyntaxLocationBlockException(const std::string &msg);
		virtual const char *what() const throw();
};
