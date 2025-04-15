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

#pragma once

#include "../ConfigFile/Server.hpp"
#include "HttpRequest.hpp"
#include "AHttp.hpp"

class Server;
class HttpRequest;

class HttpResponse : public AHttp {
	public:
		str							_line0;
		int							_status;
		void	setErrorCode(int ErrorCode, Server* server);
		void 	setResource(const HttpRequest &request, Server* server);

		public:
		HttpResponse(const HttpRequest &request, Server* server);
		HttpResponse(const HttpResponse &other);
		HttpResponse(int errorCode);
		~HttpResponse();
};
