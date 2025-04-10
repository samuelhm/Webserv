/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:28 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/09 01:19:43 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseConfig.hpp"
#include "ParseLocation.hpp"

Location *getLocation(const str &locationString, const str &serverName) {
  std::istringstream locationBlock(locationString);
  std::string line;

  std::getline(locationBlock, line);
  Logger::log(str("Parsing Location block line: ") + line, INFO);
  std::string location_path = getLocationPath(line);

  std::getline(locationBlock, line);
  if (line.compare("[") != 0) { throw BadSyntaxLocationBlockException("Not found Open Bracker ["); }

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
    else { throw BadSyntaxLocationBlockException("Failed to insert in key: " + key + " With Value: " + value + "."); }
  }

  Location* location = new Location(serverName, location_path);
  try {
    setLocationParams(location, options);
  } catch (BadOptionLocationException const &e) {
    delete location;
    std::cout << e.what() << std::endl;
    throw BadSyntaxLocationBlockException("Failed to insert Location Path." + location_path);
  }
  return location;
}

void setLocationParams(Location *location, strMap const &options) {
  for (strMap::const_iterator it = options.begin(); it != options.end(); it++) {
    if (it->first == "redirect")
      location->setRedirect(it->second);
    else if (it->first == "redirect_code")
      location->setRedirectCode(it->second);
    else if (it->first == "upload_enable") {
      if (it->second == "on")
        location->setUploadEnable(true);
    }
    else if (it->first == "autoindex") {
      if (it->second == "on")
        location->setAutoindex(true);
    }
    else if (it->first == "cgi_enable") {
      if (it->second == "on")
        location->setUploadEnable(true);
    }
    else if (it->first == "index")
      location->setIndex(it->second);
    else if (it->first == "upload_path")
      location->setUploadPath(it->second);
    else if (it->first == "cgi_extension")
      location->setCgiExtension(it->second);
    else if (it->first == "cgi_path")
      location->setCgiPath(it->second);
    else if (it->first == "allowed_methods")
      location->setMethods(it->second);
    else if (it->first == "root")
      location->setRoot(it->second);
    else if (it->first == "client_max_body_size")
      location->setBodySize(it->second);
    else {
      delete location;
      throw BadSyntaxLocationBlockException(it->first);
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
  if (!std::getline(line, tmp, ':')) { throw BadSyntaxLocationBlockException(tmp); }
  if (!std::getline(line, tmp)) { throw BadSyntaxLocationBlockException(tmp); }
  return tmp;
}

RequestType strToRequest(const str &method)
{
  Logger::log(str("Trying to insert method: ") + method, INFO);
	if (method == "POST")
		return POST;
	else if (method == "GET")
		return GET;
	else if (method == "DELETE")
		return DELETE;
	else if (method == "OPTIONS")
		return OPTIONS;
  else if (method == "PUT")
		return PUT;
  else
		throw BadOptionLocationException();
}

const char* BadOptionLocationException::what(void) const throw() { return "webserver: Bad option"; }
BadSyntaxLocationBlockException::BadSyntaxLocationBlockException(const std::string &msg) : _msg(msg) {}
const char* BadSyntaxLocationBlockException::what() const throw() { return _msg.c_str();	}
