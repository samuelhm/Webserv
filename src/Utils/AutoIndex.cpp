/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:21:03 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/27 14:49:07 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AutoIndex.hpp"
#include "AutoIndexTable.hpp"
#include "DirectoryEntry.hpp"
#include "Logger.hpp"
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>

str AutoIndex::getPrevPath(const str &path)
{
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

  strVec tHeaders(3);
  tHeaders.at(0) = "Name";
  tHeaders.at(1) = "Last Modify";
  tHeaders.at(2) = "Size";
  AutoIndexTable table(tHeaders);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
    table.addDataRow(getDirectoryEntry(entry, locationUrlPath, localPathResource));

  closedir(dir);
  
  str body(AUTOINDEXHEADER);
  body.append(table.getAutoIndexTable());
  body.append(AUTOINDEXFOOTER);
  
  return body;
}

DirectoryEntry AutoIndex::getDirectoryEntry(dirent *entry, const str &locationUrlPath, const str &localPathResource) {
  DirectoryEntry current;
  str itemPath;
  struct stat sb;

  current.d_name = entry->d_name;
  current.href = locationUrlPath + "/" + entry->d_name;
  current.d_type = entry->d_type;
  if (current.d_type == DT_DIR)
    current.d_name.append("/");
  itemPath = localPathResource + entry->d_name;
  if (stat(itemPath.c_str(), &sb) != -1) {
    current.st_size = sb.st_size;
    current.st_mtim = sb.st_mtim;
  }
  return current;
}

const char *AutoIndex::DirectoryNotAccesible::what() const throw() {
  return "No se pudo acceder al directorio.";
}
