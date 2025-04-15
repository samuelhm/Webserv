/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:35 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/15 16:52:00 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../WebSrv.hpp"
#include "../ConfigFile/Server.hpp"
#include "../ConfigFile/Location.hpp"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "../HTTP/HttpRequest.hpp"
#include "../HTTP/HttpResponse.hpp"
#include "Logger.hpp"

class Server;
class Location;
class HttpRequest;
class HttpResponse;

namespace Utils {
	extern std::map<int, str>	_statusStr;

	strVec				split(const str &input, const char delimiter);
	strVec				split(const str &input, const str &delimiter);
	str 				trim(const str& input);
	str 				fileToStr(const str &filePath);
	str 				intToStr(std::size_t num);
	void 				fillStatusStr();
	bool				setUpServers(std::vector<Server*>& servers);
	str					requestTypeToStr(RequestType type);
	void				printRequest(HttpRequest &Request);
	HttpResponse 		&codeResponse(int errorCode);

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

	template<typename T>
	bool contains(const std::vector<T>& vec, const T& valor) {
	return std::find(vec.begin(), vec.end(), valor) != vec.end();
	}

}
