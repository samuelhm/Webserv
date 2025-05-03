/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParseLocation.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 18:53:28 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/16 20:48:18 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ParseLocation.hpp"
#include "ParseConfig.hpp"

Location *getLocation(const str &locationString) {
  std::istringstream locationBlock(locationString);
  str line;

  std::getline(locationBlock, line);
  Logger::log(str("Parsing Location block line: ") + line, INFO);
  str location_path = getlocationUri(line);

  std::getline(locationBlock, line);
  if (line.compare("[") != 0) {
    throw BadSyntaxLocationBlockException("Not found Open Bracker [");
  }

  str key, value;
  std::map<str, str> options;
  while (std::getline(locationBlock, line)) {
    if (!line.compare("]")) {
      break;
    }
    if (line.empty() || line.at(0) == '#') { // Se podria sacar
      continue;                              // y pasar al parser
    } // general
    std::istringstream buffer(line);
    if (std::getline(buffer, key, ':') && std::getline(buffer, value)) {
      options[key] = value;
    } else {
      throw BadSyntaxLocationBlockException("Failed to insert in key: " + key +
                                            " With Value: " + value + ".");
    }
  }

  Location *location = new Location(location_path);
  try {
    setLocationParams(location, options);
  } catch (BadOptionLocationException const &e) {
    delete location;
    std::cout << e.what() << std::endl;
    throw BadSyntaxLocationBlockException("Failed to insert Location Path." +
                                          location_path);
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
    } else if (it->first == "autoindex") {
      if (it->second == "on")
        location->setAutoindex(true);
    } else if (it->first == "cgi_enable") {
      if (it->second == "on")
        location->setCgiEnable(true);
    } else if (it->first == "index")
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

bool isValidPath(str const &path) {
  if (path.empty())
    return false;
  if (path[0] != '/' || (path[0] != '*' && path[1] != '.'))
    return false;
  for (size_t i = 0; i < path.size(); i++) {
    if (!std::isprint(path[i]))
      return false;
    if (path[i] == ' ')
      return false;
    if (path[i] == '/' && path[i + 1] == '/')
      return false;
  }
  return true;
}

str getlocationUri(str const &locationString) {
  std::istringstream line(locationString);
  str tmp;
  if (!std::getline(line, tmp, ':')) {
    throw BadSyntaxLocationBlockException(tmp);
  }
  if (!std::getline(line, tmp)) {
    throw BadSyntaxLocationBlockException(tmp);
  }
  return tmp;
}

RequestType strToRequest(const str &method) {
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

const char *BadOptionLocationException::what(void) const throw() {
  return "webserver: Bad option";
}
BadSyntaxLocationBlockException::BadSyntaxLocationBlockException(const str &msg)
    : _msg(msg) {}
const char *BadSyntaxLocationBlockException::what() const throw() {
  return _msg.c_str();
}
