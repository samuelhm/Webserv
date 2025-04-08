#pragma once

#include "../WebSrv.hpp"
#include "../ConfigFile/Server.hpp"
#include "../ConfigFile/Location.hpp"
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>

class Server;
class Location;

namespace Utils {
	extern std::map<int, str>	_statusStr;

	std::vector<str> split(const str &input, const char delimiter);
	std::vector<str> split(const str &input, const str &delimiter);
	str trim(const str& input);
	str fileToStr(const str &filePath);
	str	intToStr(unsigned int num);
	void 	fillStatusStr();

	template <typename K, typename V> //Definir aqui o crear tpp?
	void print_map(const std::map<K, V>& m) {
		typename std::map<K, V>::const_iterator it;
		if (m.empty())
			return ;
		for (it = m.begin(); it != m.end(); ++it) {
			std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
		}
	}

	template <typename K, typename V>
	const str returnMap(const std::map<K, V>& m) {
		typename std::map<K, V>::const_iterator it;
		if (m.empty())
		return "";
		str result;
		for (it = m.begin(); it != m.end(); ++it) {
			result.append(it->first);
			result.append(":");
			result.append(it->second);
			result.append("\n");
		}
		return result;
	}

	template <typename Iterator, typename Function>
	void foreach(Iterator begin, Iterator end, Function func) {
		while (begin != end) {
			func(*begin);
			++begin;
		}
	}

	template <typename T>
	void deleteItem(T *Item) {
		delete Item;
	}
}
