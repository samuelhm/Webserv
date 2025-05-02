/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:44 by erigonza          #+#    #+#             */
/*   Updated: 2025/05/02 19:43:35 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "../WebSrv.hpp"
#include "DirectoryEntry.hpp"
#include "../ConfigFile/Location.hpp"
#include <dirent.h>

#define AUTOINDEXHEADER "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title><link rel=\"stylesheet\" href=\"http://localhost:8080/css/style.css\"></head>\n<body>\n"
#define AUTOINDEXFOOTER "</body>\n</html>"

class AutoIndex {
public:
  static str getAutoIndex(const Location &location, const str &uri);
  static str getPrevPath(const str &path);

  class DirectoryNotAccesible : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  static DirectoryEntry getDirectoryEntry(dirent *entry, const str &locationUrlPath,
                                     const str &localPathResource);
  AutoIndex();
  ~AutoIndex();
};
