#pragma once

#include "../WebSrv.hpp"
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>

namespace Utils {

	std::vector<str> split(const str &input, const char delimiter);
	std::vector<str> split(const str &input, const str &delimiter);
	str trim(const str& input);
	str fileToStr(const str &filePath);

	template <typename K, typename V> //Definir aqui o crear tpp?
	void print_map(const std::map<K, V>& m) {
		typename std::map<K, V>::const_iterator it;
		for (it = m.begin(); it != m.end(); ++it) {
			std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
		}
	}

	template <typename Iterator, typename Function>
	void foreach(Iterator begin, Iterator end, Function func) {
		while (begin != end) {
			func(*begin);
			++begin;
		}
}

}
