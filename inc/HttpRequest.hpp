/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/24 14:22:15 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>
#include "WebSrv.hpp"

class HttpRequest {
	private:
		RequestType							_type;
		std::string							_path;
		std::map<std::string, std::string>	_header;
		std::string							_body;
	public:
		HttpRequest();
		HttpRequest(const HttpRequest &other);
		HttpRequest& operator=(const HttpRequest &other);
		~HttpRequest();

		RequestType get_type() const;
		const std::string& get_path() const;
		const std::string& get_header(const std::string& key) const;
		const std::string& get_body() const;
};

#endif


