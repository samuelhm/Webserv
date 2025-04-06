#pragma once
#include <string>

enum DebugType {
	INFO,
	WARNING,
	ERROR
};

class Logger {
	public:
		static void log(const std::string& msg, DebugType type = INFO);
		static void setLevel(DebugType level);
		static void initFromEnv();

private:
	static DebugType currentLevel;
	static const char* getIcon(DebugType type);
	static const char* getColor(DebugType type);
};
