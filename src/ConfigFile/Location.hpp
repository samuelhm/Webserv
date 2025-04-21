/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: erigonza <erigonza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 10:27:46 by shurtado          #+#    #+#             */
/*   Updated: 2025/04/18 12:15:28 by erigonza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Utils/Utils.hpp"
#include "../WebSrv.hpp"

class Location {
	public:
		Location(const str &path);
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
		strVec						getCgiExtension() const;
		str							getCgiPath() const;
		str							getRedirectCode() const;
		int							getBodySize() const;
		str							getUrlPath() const;

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
		str							_urlPath;
		std::vector<RequestType>	_methods;
		str							_redirect;
		str							_redirect_code;
		bool						_uploadEnable;
		str							_root;
		bool						_autoIndex;
		str							_index;
		str							_uploadPath;
		bool						_cgiEnable;
		strVec						_cgiExtension;
		str							_cgiPath;
		int							_bodySize;
};
