/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/07 14:56:39 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "AHttp.hpp"
#include "../ConfigFile/Server.hpp"

class Server;

class HttpRequest : public AHttp {
	private:
		RequestType	_type;
		bool		_badRequest;
		str			_receivedMethod;
		str			_resource;

		void		parse();
		void		checkHeaderMRP(const str &line);
		const str	saveHeader(const str &request);
		bool 		checkResource(Server const &server);

		bool		_resorceExist;
		bool		_validMethod;
		bool		_isCgi;
		Location*	_location;

	public:
		HttpRequest(str request, Server * server);
		~HttpRequest();
		Location	*getLocation(Server* Server);

		RequestType getType() const;
		bool		getBadRequest() const;
		class badHeaderException : public std::exception
		{
			private:
				std::string _msg;
			public:
				badHeaderException(const std::string &msg);
				virtual const char *what() const throw();
				virtual ~badHeaderException() throw() {};
		};
};

#endif
