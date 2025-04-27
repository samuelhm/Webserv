/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoIndex.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fcarranz <fcarranz@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/12 13:18:44 by erigonza          #+#    #+#             */
/*   Updated: 2025/04/26 15:01:14 by fcarranz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "../WebSrv.hpp"

#define AUTOINDEXHEADER "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title></head>\n<body>\n"
#define AUTOINDEXFOOTER "</body>\n</html>"

class AutoIndex
{
	public:
		static str getAutoIndex(const str &locationUrlPath, const str &uri, const str &localPathResource);
		static str getPrevPath(const str & path);

		class DirectoryNotAccesible : public std::exception {
			public:
				virtual const char* what() const throw();
			};
	private:
	AutoIndex();
	~AutoIndex();

};
