/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 17:52:10 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <map>
#include <string>
#include "../WebSrv.hpp"

class HttpRequest {
	private:
		RequestType							_type;
		str							_path;
		std::map<str, str>	_header;
		str							_body;
	public:
		HttpRequest();
		HttpRequest(const HttpRequest &other);
		HttpRequest& operator=(const HttpRequest &other);
		~HttpRequest();

		RequestType get_type() const;
		const str& get_path() const;
		const str& get_header(const str& key) const;
		const str& get_body() const;
};

#endif


