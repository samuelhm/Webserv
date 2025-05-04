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

#pragma once

#include "../WebSrv.hpp"
#include <map>
#include <string>

class AHttp {
protected:
  str _uri;
  strMap _header;
  str _body;

public:
  AHttp();
  AHttp(str request);
  AHttp(const AHttp &other);
  AHttp &operator=(const AHttp &other);
  virtual ~AHttp() = 0;

  const str &getUri() const;
  const strMap &getHeader() const;
  const str &getBody() const;
};
