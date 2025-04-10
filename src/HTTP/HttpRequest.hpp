/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/10 12:15:35 by erigonza         ###   ########.fr       */
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
		bool		_isValidCgi;
		Location*	_location;
		str			_varCgi;
		str			_locationPath;
		str			_queryString;
		bool		_resourceExist;

	public:
		HttpRequest(str request, Server * server);
		~HttpRequest();
		Location	*findLocation(Server* Server);

		bool		checkAllowMethod();
		// void		checkIsCgi(Server *server);
		void		checkIsValidCgi();
		void		envPath(Server* server);
		bool		checkIsCgi(std::vector<str>::iterator it, std::vector<str>::iterator end, Server* server);

		//Getters
		RequestType	getType() const;
		bool		getBadRequest() const;
		bool		getResorceExist() const;
		bool		getValidMethod() const;
		bool		getIsCgi() const;
		bool		getIsValidCgi() const;
		Location*	getLocation() const;
		str			getVarCgi() const;

		//Setters
		void		setType(RequestType type);
		void		setBadRequest(bool badRequest);
		void		setResorceExist(bool resorceExist);
		void		setValidMethod(bool validMethod);
		void		setIsCgi(bool isCgi);
		void		setIsValidCgi(bool isValidCgi);
		void		setLocation(Location *location);
		void		setVarCgi(str varCgi);
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
