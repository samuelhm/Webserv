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

#include "AHttp.hpp"
#include "HttpRequest.hpp"
#include "../ConfigFile/Server.hpp"

class Server;
class HttpRequest;

class HttpResponse : public AHttp {
	public:
		str					_line0;
		int					_status;
		char				**_envp;
		char				**_argv;
		str					_cgiOutput;
		void					setErrorCode(int ErrorCode, Server* server);
		void 					setResource(const HttpRequest &request, Server* server);
		void					cgiExec(const HttpRequest &request);
		void					cgiSaveItems(const HttpRequest &request);
		void					cgiFree();

		public:
		HttpResponse(const HttpRequest &request, Server* server);
		HttpResponse(const HttpResponse &other);
		HttpResponse(int errorCode);
		~HttpResponse();
};
