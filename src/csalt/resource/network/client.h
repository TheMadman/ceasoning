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

#ifndef CSALT_RESOURCE_NETWORK_CLIENT_H
#define CSALT_RESOURCE_NETWORK_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csalt/resources.h"
#include "csalt/stores.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/**
 * \file
 * \copydoc csalt_resource_network_client
 */

/**
 * \extends csalt_static_resource
 * \brief Represents a `connect()`ed network socket.
 *
 * This represents the client side of a network program.
 */
struct csalt_resource_network_client {
	const struct csalt_static_resource_interface *vtable;
	const char *node;
	const char *service;
	const struct addrinfo *hints;

	struct csalt_store_network_client {
		const struct csalt_static_store_interface *vtable;
		int fd;
	} result;
};

/**
 * \memberof csalt_resource_network_client
 * \brief Constructs a new csalt_resource_network_client.
 *
 * \param node The node to get the address for, usually an IP
 * 	address or DNS domain name
 * \param service The service to get the address for, usually
 * 	a port number or one of the human-readable service names
 * 	in the system's "services" file, e.g. /etc/services
 * \param hints An addrinfo struct, used to narrow down the
 * 	types of addresses to query
 *
 * \returns The new csalt_resource_network_client resource
 */
struct csalt_resource_network_client csalt_resource_network_client(
	const char *node,
	const char *service,
	const struct addrinfo *hints
);
csalt_static_store *csalt_resource_network_client_init(
	csalt_static_resource *resource
);
void csalt_resource_network_client_deinit(csalt_resource *resource);

/**
 * \public \memberof csalt_store_network_client
 * \brief Implements the `sendto` system call for
 * 	network stores.
 *
 * \param network The network store to send on
 * \param buffer The data to send
 * \param length The amount of data to send
 * \param flags The flags to pass to the sendto call
 * \param dest_addr The address to send the data to
 * \param addrlen The length of the address struct pointed to
 * 	by dest_addr
 *
 * \returns The amount sent with this call
 */
ssize_t csalt_store_network_client_sendto(
	const struct csalt_store_network_client *network,
	const void *buffer,
	ssize_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addrlen
);

/**
 * \public \memberof csalt_store_network_client
 * \brief Implements the `recvfrom` system call for
 * 	network stores.
 *
 * \param network The network store to receive from
 * \param buffer The memory to store the received data to
 * \param length The amount of data to receive
 * \param flags The flags to pass to the recvfrom call
 * \param src_addr The location to store the address to
 * \param addrlen The length of the address struct pointed to
 * 	by src_addr
 *
 * \returns The amount received with this call
 */
ssize_t csalt_store_network_client_recvfrom(
	const struct csalt_store_network_client *network,
	void *buffer,
	ssize_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
);

ssize_t csalt_store_network_client_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
);
ssize_t csalt_store_network_client_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
);
int csalt_store_network_client_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCE_NETWORK_CLIENT_H
