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

class HttpResponse : public AHttp {
	public:
		HttpResponse(const HttpRequest &request);
		HttpResponse(const HttpResponse &other);
		~HttpResponse();
};

#endif


