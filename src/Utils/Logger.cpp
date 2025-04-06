#include "Logger.hpp"
#include <iostream>
#include <cstdlib> // std::getenv

DebugType Logger::currentLevel = INFO;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

void Logger::setLevel(DebugType level) {
	currentLevel = level;
}

void Logger::initFromEnv() {
	const char* env = std::getenv("DEBUG_LEVEL");
	if (!env)
		return;

	std::string lvl = env;
	if (lvl == "INFO")        setLevel(INFO);
	else if (lvl == "WARNING") setLevel(WARNING);
	else if (lvl == "ERROR")   setLevel(ERROR);
}

void Logger::log(const std::string& msg, DebugType type) {
	if (static_cast<int>(type) < static_cast<int>(currentLevel))
		return;

	std::cout << getColor(type) << getIcon(type) << msg << RESET << std::endl;
}

const char* Logger::getIcon(DebugType type) {
	switch (type) {
		case INFO:    return "🔵";
		case WARNING: return "🟡";
		case ERROR:   return "🔴";
		default:                 return "";
	}
}

const char* Logger::getColor(DebugType type) {
	switch (type) {
		case INFO:
			return BLUE;
		case WARNING:
			return YELLOW;
		case ERROR:
			return RED;
		default:
			return RESET;
	}
}
