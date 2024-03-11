/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2024   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSALT_RESOURCE_NETWORK_H
#define CSALT_RESOURCE_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csalt/resources.h"
#include "csalt/stores.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "network/client.h"

/**
 * \file
 * \brief This module contains common networking functionality
 * 	used by most network resources.
 */

/**
 * \brief Provides an alternative interface for getaddrinfo.
 *
 * This interface removes the need to perform resource management
 * for the getaddrinfo resource returned, by instead using a
 * callback.
 *
 * If there is a getaddrinfo error, the callback is not called.
 *
 * \param node The node to get the address for, usually an IP
 * 	address or DNS domain name
 * \param service The service to get the address for, usually
 * 	a port number or one of the human-readable service names
 * 	in the system's "services" file, e.g. /etc/services
 * \param hints An addrinfo struct, used to narrow down the
 * 	types of addresses to query
 * \param callback A function for using the resulting
 * 	addrinfo linked list
 * \param param An additional parameter to pass to the callback
 *
 * \returns The return value of callback on success, otherwise
 * 	an error code from getaddrinfo.
 */
int csalt_resource_network_getaddrinfo(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	int (*callback)(const struct addrinfo *result, void *param),
	void *param
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCE_NETWORK_H
