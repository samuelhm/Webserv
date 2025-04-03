/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 16:14:49 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "AHttp.hpp"
#include "HttpRequest.hpp"
#include "../ConfigFile/Server.hpp"

class HttpResponse : public AHttp {
	public:
		str							_line0;
		int							_status;
		static std::map<int, str>	_statusStr;
		void	setErrorCode(int ErrorCode, Server* server);
		void 	fillStatusStr();

		public:
		HttpResponse(const HttpRequest &request, Server* server);
		HttpResponse(const HttpResponse &other);
		~HttpResponse();
};

#endif


