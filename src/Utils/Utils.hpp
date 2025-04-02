#pragma once

#include "../WebSrv.hpp"
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>

namespace Utils {

	std::vector<str> split(const str &input, char delimiter);
	std::vector<str> split(const str &input, const str &delimiter);
	str trim(const str& input);

	template <typename K, typename V> //Definir aqui o crear tpp?
	void print_map(const std::map<K, V>& m) {
		typename std::map<K, V>::const_iterator it;
		for (it = m.begin(); it != m.end(); ++it) {
			std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
		}
	}

}
