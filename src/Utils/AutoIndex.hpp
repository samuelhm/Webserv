/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:44 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/27 14:46:26 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "../WebSrv.hpp"
#include "DirectoryEntry.hpp"
#include <string>
#include <dirent.h>

#define AUTOINDEXHEADER "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title></head>\n<body>\n"
#define AUTOINDEXFOOTER "</body>\n</html>"

class AutoIndex {
public:
  static str getAutoIndex(const str &locationUrlPath, const str &uri, const str &localPathResource);
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
