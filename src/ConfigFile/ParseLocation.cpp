/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:28 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/06 16:36:06 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseConfig.hpp"
#include "ParseLocation.hpp"

Location *getLocation(const str &locationString, const str &serverName) {
  std::istringstream locationBlock(locationString);
  std::string line;

  std::getline(locationBlock, line);
  std::string location_path = getLocationPath(line);

  std::getline(locationBlock, line);
  if (line.compare("[") != 0) { throw BadSyntaxLocationBlockException(); }

  std::string key, value;
  std::map<std::string, std::string> options;
  while (std::getline(locationBlock, line)) {
    if (!line.compare("]")) { break; }
    if (line.empty() || line.at(0) == '#') {   // Se podria sacar
      continue;                                // y pasar al parser
    }                                          // general
    std::istringstream buffer(line);
    if (std::getline(buffer, key, ':') && std::getline(buffer, value)) {
      options[key] = value;
    }
    else { throw BadSyntaxLocationBlockException(); }
  }

  Location* location = new Location(serverName, location_path);
  try {
    setLocationParams(location, options);
  } catch (BadOptionLocationException const &e) {
    delete location;
    std::cout << e.what() << std::endl;
    throw BadSyntaxLocationBlockException();
  }
  // Delete block
  std::cout << "\n====== Server: " << serverName << " Location path: "
            << location->getRoot() << " ======" << std::endl
            << "redirect: \t" << location->getRedirect() << std::endl
            << "redirect_code: \t" << location->getRedirectCode() << std::endl
            << "uploadEnable: \t" << ((location->getUploadEnable()) ? "true" : "false")
            << std::endl
            << "autoIndex: \t" << ((location->getAutoindex()) ? "true" : "false")
            << std::endl
            << "index: \t\t" << location->getIndex() << std::endl
			<< "uploadPath: \t" << location->getUploadPath() << std::endl
			<< "cgiEnable: \t" << ((location->getCgiEnable()) ? "true" : "false")
            << std::endl
			<< "cgiExtension: \t" << location->getCgiExtension() << std::endl
			<< "cgiPath: \t" << location->getCgiPath() << std::endl;
  // End block
  return location;
}

void setLocationParams(Location *location, std::map<str, str> const &options) {
  for (std::map<str, str>::const_iterator it = options.begin(); it != options.end(); it++) {
    if (it->first == "redirect")
      location->setRedirect(it->second);
    else if (it->first == "redirect_code")
      location->setRedirectCode(it->second);
    else if (it->first == "uploadEnable") {
      if (it->second == "on")
        location->setUploadEnable(true);
    }
    else if (it->first == "autoindex") {
      if (it->second == "on")
        location->setAutoindex(true);
    }
    else if (it->first == "cgiEnable") {
      if (it->second == "on")
        location->setUploadEnable(true);
    }
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

RequestType strToRequest(const str &method)
{
	if (method == "POST")
		return POST;
	else if (method == "GET")
		return GET;
	else if (method == "DELETE")
		return DELETE;
	else if (method == "OPTIONS")
		return OPTIONS;
	else
		throw BadOptionLocationException();
}

const char* BadOptionLocationException::what(void) const throw() { return "webserver: Bad option"; }
const char* BadSyntaxLocationBlockException::what(void) const throw() { return "webserver: configuration file failed! Bad syntax on location block"; }
