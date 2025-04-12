/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:21:18 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/12 13:39:13 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <iostream>
#include <cstdlib> // std::getenv
#include <string>

DebugType Logger::currentLevel = INFO;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define BOLD_RED "\x1B[1m\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define BOLD "\x1B[1m"
#define ITAL "\x1B[3m"
#define UNDL "\x1B[4m"

#define B_RED "\x1B[41m"
#define B_GRN "\x1B[42m"
#define B_YLW "\x1B[43m"
#define B_BLU "\x1B[44m"
#define B_MAG "\x1B[45m"
#define B_CYN "\x1B[46m"
#define B_WHI "\x1B[47m"

#define BOLDYLW "\x1B[1m\033[1;33m"


void Logger::setLevel(DebugType level) {
	currentLevel = level;
}

void Logger::initFromEnv() {
	const char* env = std::getenv("DEBUG_LEVEL");
	str lvl;
	if (!env)
		lvl = "WARNING";
	else
		lvl = env;
	if (lvl == "INFO")         setLevel(INFO);
	else if (lvl == "USER")    setLevel(USER);
	else if (lvl == "WARNING") setLevel(WARNING);
	else if (lvl == "ERROR")   setLevel(ERROR);
}

void Logger::log(const str& msg, DebugType type) {
	if (type < currentLevel)
		return;

	std::cout << getColor(type) << getIcon(type) << msg << RESET << std::endl;
}

const char* Logger::getIcon(DebugType type) {
	switch (type) {
		case INFO:    return "🔵";
		case USER:    return "🟢";
		case WARNING: return "🟡";
		case ERROR:   return "🔴";
		default:                 return "";
	}
}


const char* Logger::getColor(DebugType type) {
	switch (type) {
		case INFO:
			return CYAN;
		case WARNING:
			return BOLDYLW;
		case ERROR:
			return BOLD_RED;
		case USER:
			return GREEN;
		default:
			return RESET;
	}
}
