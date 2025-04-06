/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:28 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/06 14:58:06 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseConfig.hpp"
#include "ParseLocation.hpp"


void setLocationParams(Location *location, std::map<str, str> const &options)
{
  for (std::map<str, str>::const_iterator it = options.begin(); it != options.end(); it++) {
    if (it->first == "redirect")
        location->setRedirect(it->second);
    else if (it->first == "Redirect_code")
        location->setRedirectCode(it->second);
    else if (it->first == "uploadEnable" && it->second == "on")
        location->setUploadEnable(true);
    else if (it->first == "autoIndex" && it->second == "on")
        location->setAutoindex(true);
    else if (it->first == "cgiEnable" && it->second == "on")
        location->setUploadEnable(true);
    else if (it->first == "index")
        location->setIndex(it->second);
    else if (it->first == "uploadPath")
        location->setUploadPath(it->second);
    else if (it->first == "cgiExtension")
        location->setCgiExtension(it->second);
    else if (it->first == "cgiPath")
        location->setCgiPath(it->second);
    else if (it->first == "allowed_methods")
        location->setMethods(it->second);
    else if (it->first == "root")
        location->setRoot(it->second);
	else {
        if (it->first == "autoIndex" || it->first == "uploadEnable" || it->first == "cgiEnable")
          continue ;
        delete location;
        throw BadSyntaxLocationBlockException();
    }
  }
}

bool isValidPath(std::string const &path) {
	if (path.empty()) return false;
	if (path[0] != '/' || (path[0] != '*' && path[1] != '.')) return false;
	for (size_t i = 0; i < path.size(); i++) {
	  if (!std::isprint(path[i])) return false;
	  if (path[i] == ' ') return false;
	  if (path[i] == '/' && path[i + 1] == '/') return false;
	}
	return true;
  }

  std::string getLocationPath(std::string const &locationString) {
	std::istringstream line(locationString);
	std::string tmp;
	if (!std::getline(line, tmp, ':')) { throw BadSyntaxLocationBlockException(); }
	if (!std::getline(line, tmp)) { throw BadSyntaxLocationBlockException(); }
	return tmp;
  }
