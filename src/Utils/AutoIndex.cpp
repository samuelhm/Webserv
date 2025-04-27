/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:21:03 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/27 10:58:46 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AutoIndex.hpp"
#include "AutoIndexTable.hpp"
#include "Logger.hpp"
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>

str AutoIndex::getPrevPath(const str &path) {
  if (path.empty())
    return path;
  str ruta = path;
  if (ruta[ruta.size() - 1] == '/')
    ruta.erase(ruta.size() - 1);
  std::size_t pos = ruta.find_last_of('/');
  if (pos == str::npos)
    return "/";
  return ruta.substr(0, pos + 1);
}

std::string getDirectoryToOpen(const str &locationUrlPath, const str &uri, const str &localPathResourceconst) {
  std::size_t start;
  
  start = uri.find(locationUrlPath);
  start += locationUrlPath.size() + 1;
  
  return std::string(localPathResourceconst + uri.substr(start));
}

str AutoIndex::getAutoIndex(const str &locationUrlPath, const str &uri, const str &localPathResource) {

  std::string dirToOpen = getDirectoryToOpen(locationUrlPath, uri, localPathResource);
  DIR *dir = opendir(dirToOpen.c_str());
  if (!dir)
    throw DirectoryNotAccesible();
  strVec theaders(3);
  theaders.at(0) = "Name";
  theaders.at(1) = "Last Modify";
  theaders.at(2) = "Size";
  AutoIndexTable table(theaders);

  struct dirent *entry;
  str itemPath;
  
  struct stat sb;
  dirItemInfo current;
  while ((entry = readdir(dir)) != NULL) {
    current.d_name = entry->d_name;
    current.href = locationUrlPath + "/" + entry->d_name;
    current.d_type = entry->d_type;
    itemPath = localPathResource + entry->d_name;
    if (stat(itemPath.c_str(), &sb) == -1) {
      break;
    }
    current.st_size = sb.st_size;
    current.st_mtim = sb.st_mtim;
    table.addDataRow(current);
    current.clear();
  }
  closedir(dir);
  
  str body(AUTOINDEXHEADER);
  body.append(table.getAutoIndexTable());
  body.append(AUTOINDEXFOOTER);
  
  return body;
}

const char *AutoIndex::DirectoryNotAccesible::what() const throw() {
  return "No se pudo acceder al directorio.";
}
