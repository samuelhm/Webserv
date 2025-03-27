/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHttp.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 15:18:16 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "AHttp.hpp"

AHttp::AHttp() {}

AHttp::AHttp(str request) {}

AHttp::AHttp(const AHttp &other) { *this = other; }

AHttp& AHttp::operator=(const AHttp &other) {

	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

AHttp::~AHttp() {

}


