/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: shurtado <shurtado@student.42barcelona.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/25 11:44:20 by shurtado          #+#    #+#             */
/*   Updated: 2025/03/25 12:25:57 by shurtado         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFile/Server.hpp"

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int ac, char **av)
{
	(void)ac;
	(void)av;
	Server servidor;
	struct addrinfo a;

	return 0;
}
