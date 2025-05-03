/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndexTable.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcelona..>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/27 12:23:53 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/30 22:33:53 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AutoIndexTable.hpp"
#include "DirectoryEntry.hpp"
#include "../WebSrv.hpp"
#include <cstddef>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>

AutoIndexTable::AutoIndexTable(void) {}
AutoIndexTable::AutoIndexTable(AutoIndexTable const &other) { (void)other; }
AutoIndexTable &AutoIndexTable::operator=(AutoIndexTable const &other) {
  (void)other; return *this; }
AutoIndexTable::~AutoIndexTable(void) {}


AutoIndexTable::AutoIndexTable(strVec const &th) : _headerRow(th.size()) {
  for (std::size_t i = 0; i < th.size(); i++) {
    _headerRow[i] = th[i];
  }
}

const std::string AutoIndexTable::getTableHeader(void) {
  std::ostringstream oss;

  oss << "<thead>\n<tr>\n";
  for (std::size_t i = 0; i != _headerRow.size(); i++)
    oss << "<th>" << _headerRow.at(i) << "</th>\n";
  oss << "</tr>\n</thead>\n";

  return oss.str();
}

const std::string AutoIndexTable::getTableRows(void) {
  std::ostringstream oss;

  oss << "<tbody>\n";
  for (std::vector<DirectoryEntry>::const_iterator it = _dataRow.begin(); it != _dataRow.end(); it++) {
    oss << "<tr>\n"
        << "<td>" << getHtmlLink(*it) << "</td>\n"
        << "<td>" << getTimeString(it->st_mtim) << "</td>\n"
        << "<td>" << it->st_size << "</td>\n"
        << "</tr>\n";
  }
  oss << "</tbody>\n";

  return oss.str();
}

const std::string AutoIndexTable::getAutoIndexTable(void) {

  std::string table("<table>\n");
  table.append(getTableHeader());
  table.append(getTableRows());
  table.append("</table>\n");

  return table;
}


std::string AutoIndexTable::getHtmlLink(DirectoryEntry const &dataRowInfo) {
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

void AutoIndexTable::addDataRow(DirectoryEntry const &dEntry) {
  std::size_t index = 0;

  while (index < _dataRow.size()) {
    if (dEntry.d_type == DT_DIR && _dataRow[index].d_type != DT_DIR)
      break;
    if (dEntry.d_type == _dataRow[index].d_type && dEntry.d_name.compare(_dataRow[index].d_name) < 0)
      break;
    ++index;
  }
  _dataRow.insert(_dataRow.begin() + index, dEntry);
}
