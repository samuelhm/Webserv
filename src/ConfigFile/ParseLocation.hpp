/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:08 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/12 13:39:13 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../HTTP/HttpResponse.hpp"
#include "../WebSrv.hpp"
#include "Location.hpp"
#include "ParseConfig.hpp"

void setLocationParams(Location *location, strMap const &options);
bool isValidPath(str const &path);
str getlocationUri(str const &locationString);
Location *getLocation(const str &locationString);
RequestType strToRequest(const str &method);

class BadOptionLocationException : public std::exception {
public:
  virtual const char *what() const throw();
};
class BadSyntaxLocationBlockException : public std::exception {
private:
  str _msg;

public:
  virtual ~BadSyntaxLocationBlockException(void) throw() {}
  BadSyntaxLocationBlockException(const str &msg);
  virtual const char *what() const throw();
};
