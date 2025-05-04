/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:44 by erigonza          #+#    #+#             */
/*   Updated: 2025/05/04 19:46:17 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../ConfigFile/Location.hpp"
#include "../WebSrv.hpp"
#include "DirectoryEntry.hpp"
#include <dirent.h>

#define AUTOINDEXHEADER                                                        \
  "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title><link "  \
  "rel=\"stylesheet\" "                                                        \
  "href=\"http://localhost:8080/css/style.css\"></head>\n<body>\n"
#define AUTOINDEXFOOTER "</body>\n</html>"

class Location;

class AutoIndex {
public:
  static str getAutoIndex(const Location &location, const str &uri);
  static str getPrevPath(const str &path);

  class DirectoryNotAccesible : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  static DirectoryEntry getDirectoryEntry(dirent *entry,
                                          const Location &location,
                                          const str &resourceDirectory);
  AutoIndex();
  ~AutoIndex();
};
