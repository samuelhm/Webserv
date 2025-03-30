/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/30 16:53:49 by shurtado         ###   ########.fr       */
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
		HttpRequest(const HttpRequest &other);
		~HttpRequest();

		RequestType get_type() const;
		class badHeaderException : public std::exception
		{
			public:
				const char *what() const throw();
		};
};

#endif


