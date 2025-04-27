/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndexTable.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/12 00:19:03 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/27 12:34:31 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>

struct dirItemInfo {
  std::string d_name;
  std::string href;
  unsigned char d_type;
  off_t st_size;
  struct timespec st_mtim;

  void clear();
};

class AutoIndexTable {

typedef std::vector<std::string> vStr;

public:
  AutoIndexTable(vStr const &th);

	~AutoIndexTable(void);

	const std::string getAutoIndexTable(void);
	void addDataRow(dirItemInfo const &dataRowInfo);

private:
	vStr _headerRow;
	std::vector<vStr> _dataRow;

  std::string getHtmlLink(dirItemInfo const &dataRowInfo);
  std::string getTimeString(struct timespec const &time);

	AutoIndexTable(void);
	AutoIndexTable(AutoIndexTable const &other);
	AutoIndexTable &operator=(AutoIndexTable const &other);
};
