/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHttp copy.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:12:09 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/27 16:08:49 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef AHTTP_HPP
#define AHTTP_HPP

#include <map>
#include <string>
#include "../WebSrv.hpp"

class AHttp {
	protected:
		str							_path;
		std::map<str, str>			_header;
		str							_body;
	public:
		AHttp();
		AHttp(str request);
		AHttp(const AHttp &other);
		AHttp& operator=(const AHttp &other);
		virtual ~AHttp() = 0;

		const str& get_path() const;
		const str& get_header(const str& key) const;
		const str& get_body() const;
};

#endif


