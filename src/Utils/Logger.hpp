/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:40 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/12 13:41:04 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../WebSrv.hpp"
#include <string>

enum DebugType { INFO, USER, WARNING, ERROR };

class Logger {
public:
  static void log(const std::string &msg, DebugType type = INFO);
  static void setLevel(DebugType level);
  static void initFromEnv();

private:
  static DebugType currentLevel;
  static const char *getIcon(DebugType type);
  static const char *getColor(DebugType type);
};
