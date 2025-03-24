/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:11:54 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/24 13:14:09 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "HttpRequest.hpp"

HttpRequest::HttpRequest() {}

HttpRequest::HttpRequest(const HttpRequest &other) { *this = other; }

HttpRequest& HttpRequest::operator=(const HttpRequest &other) {

	if (this != &other) {
		// Copiar los atributos necesarios
	}
	return *this;
}

HttpRequest::~HttpRequest() {

}


