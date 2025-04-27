/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndexTable.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcelona..>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/27 12:23:53 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/27 12:23:58 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AutoIndexTable.hpp"
#include "../WebSrv.hpp"
#include <cstring>
#include <sstream>
#include <dirent.h>

void dirItemInfo::clear() {
  d_name.clear();
  href.clear();
  d_type = 0;
  st_size = 0;
  st_mtim.tv_nsec = 0;
  st_mtim.tv_sec = 0;
}

AutoIndexTable::AutoIndexTable(void) {}
AutoIndexTable::AutoIndexTable(AutoIndexTable const &other) { (void)other; }
AutoIndexTable &AutoIndexTable::operator=(AutoIndexTable const &other)
{ (void)other; return *this; }


AutoIndexTable::AutoIndexTable(vStr const &th) : _headerRow(th.size()) {
  for (std::size_t i = 0; i < th.size(); i++) {
    _headerRow[i] = th[i];
  }
}

AutoIndexTable::~AutoIndexTable(void) {}

const std::string AutoIndexTable::getAutoIndexTable(void) {
  std::ostringstream oss;

  oss << "<table>\n<thead>\n<tr>\n";
  for (std::size_t i = 0; i != _headerRow.size(); i++) {
    oss << "<th>" << _headerRow.at(i) << "</th>\n";
  }
  oss << "</tr>\n</thead>\n";

  oss << "<tbody>\n";
  for (std::vector<vStr>::const_iterator it = _dataRow.begin();
       it != _dataRow.end(); it++) {
    oss << "<tr>\n";
    for (std::size_t i = 0; i != _headerRow.size(); i++) {
      oss << "<td>" << it->at(i) << "</td>\n";
    }
    oss << "</tr>\n";
  }
  oss << "</tbody>\n</table>\n";

  return oss.str();
}


std::string AutoIndexTable::getHtmlLink(dirItemInfo const &dataRowInfo) {
  std::ostringstream oss;
  oss << "<a href=\"" << dataRowInfo.href << "\">" << dataRowInfo.d_name << "</a>";
  return oss.str();
}

std::string AutoIndexTable::getTimeString(struct timespec const &time) {
  char buffer[30];
  memset(&buffer, 0, sizeof(buffer));
  strftime(buffer, sizeof(buffer), "%b %d %Y %H:%M:%S", localtime(&time.tv_sec));
  return std::string(buffer);
}

void AutoIndexTable::addDataRow(dirItemInfo const &dataRowInfo) {
  std::string link, lastModify, size;

  link = getHtmlLink(dataRowInfo);
  lastModify = getTimeString(dataRowInfo.st_mtim);

  std::ostringstream oss;
  oss << dataRowInfo.st_size;
  size = oss.str();

  strVec tableRowItems;
  tableRowItems.push_back(link);
  tableRowItems.push_back(lastModify);
  tableRowItems.push_back(size);

  _dataRow.push_back(tableRowItems);
}
