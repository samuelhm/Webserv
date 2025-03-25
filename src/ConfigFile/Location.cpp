/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:10 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 11:42:42 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(const std::string &serverName) {

	_methods.push_back(GET);
	_uploadEnable = false;
	_autoindex = false;
	_index = "index.html";
	_uploadPath = "./" + serverName;
}

Location::Location(const Location &other) {
	// Constructor de copia
	*this = other;
}

Location& Location::operator=(const Location &other) {
	// Operador de asignación
	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

Location::~Location() {
	// Destructor
}
