/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/31 13:29:25 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "AHttp.hpp"

class HttpRequest : public AHttp {
	private:
		RequestType	_type;
		bool		_badRequest;
		void		parse();
		void		checkHeaderMRP(const str &line);
		const str	saveHeader(const str &request);
	public:
		HttpRequest(str request);
		~HttpRequest();

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
