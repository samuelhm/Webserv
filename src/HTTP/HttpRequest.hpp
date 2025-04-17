/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/17 13:52:01 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../ConfigFile/Server.hpp"
#include "AHttp.hpp"

class Server;
class Location;

class HttpRequest : public AHttp {
	private:
		RequestType	_type;
		bool		_badRequest;
		str			_receivedMethod;
		str			_resource;
		bool		_resourceExists;
		bool		_validMethod;
		bool		_isCgi;
		bool		_isValidCgi;
		str			_localPathResource;
		Location*	_location;
		str			_locationUri;
		str			_queryString;
		str			_pathInfo;
		bool		_headerTooLarge; // Para cabeceras mayores a LIMIT_HEADER_SIZE // Se podria mover a AHttp?
		bool		_redirect;

		void		parse();
		void		checkHeaderMRP(const str &line);
		const str	saveHeader(const str &request);
		bool 		checkResource(Server const &server);


	public:
		HttpRequest(str request, Server * server);
		~HttpRequest();
		Location	*findLocation(Server* Server);
		Location*	findLocation(Server* Server, const str &uri);

		bool		checkAllowMethod();
		// void		saveUri(Server *server);
		void		checkIsValidCgi();
		void		envPath(Server* server);

		bool		saveUri(strVecIt it, strVecIt end, Server* server);
		bool		checkValidCgi(strVecIt it, Location *loc);
		void		saveScriptNameAndQueryString(strVecIt it, strVecIt end);
		void		addPathInfo(strVecIt it, strVecIt end);
		void		autoIndex(Location *loc);


		//Getters
		RequestType	getType() const;
		bool		getBadRequest() const;
		bool		getResourceExists() const;
		bool		getValidMethod() const;
		bool		getIsCgi() const;
		bool		getIsValidCgi() const;
		Location*	getLocation() const;
		str			getVarCgi() const;
		bool		getHeaderTooLarge() const;
		str			getReceivedMethod() const;
		str			getResource() const;
		str			getLocalPathResource() const;
		str			getLocationUri() const;
		str			getQueryString() const;
		str			getPathInfo() const;
		bool		getRedirect() const;

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
				str _msg;
			public:
				badHeaderException(const str &msg);
				virtual const char *what() const throw();
				virtual ~badHeaderException() throw() {};
		};
};
