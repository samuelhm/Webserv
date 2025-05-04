/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryEntry.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcelona..>  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/27 12:24:51 by fcarranz          #+#    #+#             */
/*   Updated: 2025/04/27 14:45:37 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <sys/time.h>
#include <sys/types.h>

struct DirectoryEntry {
  std::string d_name;
  std::string href;
  unsigned char d_type;
  off_t st_size;
  struct timespec st_mtim;
};
