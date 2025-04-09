/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/09 10:15:32 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../WebSrv.hpp"
#include <vector>
#include <string>
#include "../Utils/Utils.hpp"
#include <cstdlib>

class Location {
	public:
		Location(const str &serverName, const str &path);
		~Location();

		//Getters
		std::vector<RequestType>	getMethods() const;
		str							getRedirect() const;
		bool						getUploadEnable() const;
		str							getRoot() const;
		bool						getAutoindex() const;
		str							getIndex() const;
		str							getUploadPath() const;
		bool						getCgiEnable() const;
		str							getCgiExtension() const;
		str							getCgiPath() const;
		str							getRedirectCode() const;
		int							getBodySize() const;

		//Setters
		void						setMethods(const str &methods);
		void						setRedirect(str redirect);
		void						setUploadEnable(bool uploadEnable);
		void						setRoot(str root);
		void						setAutoindex(bool autoIndex);
		void						setIndex(str index);
		void						setUploadPath(str uploadPath);
		void						setCgiEnable(bool cgiEnable);
		void						setCgiExtension(str cgiExtension);
		void						setCgiPath(str cgiPath);
		void						setRedirectCode(const str &code);
		void						setBodySize(const str &size);

	private:
		std::vector<RequestType>	_methods;
		str							_redirect;
		str							_redirect_code;
		bool						_uploadEnable;
		str							_root;
		bool						_autoIndex;
		str							_index;
		str							_uploadPath;
		bool						_cgiEnable;
		str							_cgiExtension;
		str							_cgiPath;
		int							_bodySize;

};
