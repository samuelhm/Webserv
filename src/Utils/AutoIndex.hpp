/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:44 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/12 13:20:56 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "../WebSrv.hpp"

#define AUTOINDEXHEADER "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title></head>\n<body>\n<ul>\n"
#define AUTOINDEXFOOTER "</ul></body></html>"

class AutoIndex
{
	public:
		static str getAutoIndex(const str &path);
		static str getPrevPath(const str & path);

		class DirectoryNotAccesible : public std::exception {
			public:
				virtual const char* what() const throw();
			};
	private:
	AutoIndex();
	~AutoIndex();

};
