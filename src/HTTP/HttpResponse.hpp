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
#include <fstream>
#include "../EventPool/EventPool.hpp"


class Server;
class HttpRequest;
struct eventStructTmp;

class HttpResponse : public AHttp {
	public:
		str					_line0;
		int					_status;
		char				**_envp;
		char				**_argv;
		str					_cgiOutput;
		bool				_cgiSaveErr;
		bool				_cgiNonBlock;
		void					setErrorCode(int ErrorCode, Server* server);
		void					cgiExec(const HttpRequest &request, eventStructTmp *eventStrct, int pollFd);
		void					cgiSaveItems(const HttpRequest &request);
		void					cgiFree();
		void					staticFileExec(const HttpRequest &request, Server* server);
		void					staticFileGet(const HttpRequest &request, Server* server);
		void					staticFilePost(const HttpRequest &request, Server* server);
		void					staticFilePut(const HttpRequest &request, Server* server);
		void					staticFileOptions(const HttpRequest &request, Server* server);
		void					staticFileDelete(const HttpRequest &request, Server* server);
		void					redirecResponse(const HttpRequest &request, Server* server);
		str						saveCgiHeader(const str cgiOutput);
		void					safeCloseCgiExec(Server *server, const str &msg);

		public:
		HttpResponse();
		HttpResponse(const HttpRequest &request, Server* server);
		HttpResponse(const HttpRequest &request, eventStructTmp *eventStrct, int pollFd);
		HttpResponse(int errorCode, Server *server);
		HttpResponse(const HttpRequest &request, Server* server, str autoIndexHTML);
		~HttpResponse();
};
