/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndexTable.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:19:03 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/27 14:44:27 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "DirectoryEntry.hpp"
#include "../WebSrv.hpp"
#include <string>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>

class AutoIndexTable {

public:
  AutoIndexTable(strVec const &th);
  ~AutoIndexTable(void);

  const std::string getAutoIndexTable(void);
  void addDataRow(DirectoryEntry const &dataRowInfo);

private:
  strVec _headerRow;
  std::vector<DirectoryEntry> _dataRow;

  std::string getHtmlLink(DirectoryEntry const &dataRowInfo);
  std::string getTimeString(struct timespec const &time);
  const std::string getTableHeader(void);
  const std::string getTableRows(void);

  AutoIndexTable(void);
  AutoIndexTable(AutoIndexTable const &other);
  AutoIndexTable &operator=(AutoIndexTable const &other);
};
