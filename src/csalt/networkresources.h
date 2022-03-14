/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
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

#ifndef CSALT_NETWORKRESOURCES_H
#define CSALT_NETWORKRESOURCES_H

#include <csalt/platform/init.h>

#include "baseresources.h"

#include <csalt/platform/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 * \brief Provides resources for networking
 */

typedef struct csalt_resource_network_socket_initialized csalt_resource_network;

typedef ssize_t csalt_resource_sendto_fn(
	csalt_resource_network *network,
	const void *buffer,
	ssize_t length,
	int flags,

	// Looks like Windows uses a `struct sockaddr` as well?
	const struct sockaddr *dest_addr,
	socklen_t addr_t
);

/**
 * \brief Function wrapping the sendto() network socket call.
 *
 * This function is primarily useful for unconnected sockets, such
 * as a server-side UDP socket or peer-to-peer UDP socket.
 *
 * Connected UDP sockets, and other kinds of network sockets
 * (including TCP client sockets), should use the generic
 * read/write functions, and can safely be incorporated into
 * composite resources as a consequence.
 */
ssize_t csalt_resource_sendto(
	csalt_resource_network *network,
	const void *buffer,
	ssize_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
);

typedef ssize_t csalt_resource_recvfrom_fn(
	csalt_resource_network *network,
	void *buffer,
	ssize_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
);

/**
 * \brief Function wrapping the recvfrom() network socket call.
 *
 * This function is primarily useful for unconnected sockets, such
 * as a server-side UDP socket or peer-to-peer UDP socket.
 *
 * Connected UDP sockets, and other kinds of network sockets
 * (including TCP client sockets), should use the generic
 * read/write functions, and can safely be incorporated into
 * composite resources as a consequence.
 */
ssize_t csalt_resource_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	ssize_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
);

struct csalt_resource_network_initialized_interface {
	csalt_resource_sendto_fn *sendto;
	csalt_resource_recvfrom_fn *recvfrom;
};

/**
 * \brief Represents the common features of each kind of network socket.
 */
struct csalt_resource_network_socket_initialized {
	struct csalt_store_interface *vtable;
	struct csalt_resource_network_initialized_interface *network_vtable;
	int fd;
	int domain;
	int type;
	int protocol;
};

/**
 * \brief This structure represents a UDP socket.
 *
 * Network sockets do not support csalt_store_split() - calling
 * csalt_store_split() on a network socket will pass the original
 * store to the callback.
 * 
 * Creating a connected socket with csalt_resource_network_udp_connected()
 * enables csalt_store_read() and csalt_store_write() functionality,
 * and such a socket can be used in abstract data types without issue.
 * As is the case with the rest of the library, nonblocking sockets
 * are implied. csalt_store_read() will return 0 bytes in the case of
 * the socket having no data to read.
 *
 * Creating a bound socket with csalt_resource_network_udp_bound()
 * enables only csalt_store_read(). The generic store interface isn't
 * very useful for this kind of socket, however, as it loses information
 * about who sent the packet. The primary benefit of this interface in this
 * context, then, is the resource interface and csalt_resource_use().
 * csalt_resource_sendto() and csalt_resource_recvfrom() are provided as
 * wrappers around those functions.
 *
 * Creating an unconnected, unbound socket with csalt_network_udp_stateless()
 * does not enable csalt_store_read() or csalt_store_write().
 *
 * \see csalt_resource_network_udp_connected()
 * \see csalt_resource_network_udp_bound()
 * \see csalt_resource_network_udp_stateless()
 */
struct csalt_resource_network_socket {
	struct csalt_resource_interface *vtable;
	const char *node;
	const char *service;
	struct csalt_resource_network_socket_initialized udp;
};

/**
 * \brief Creates a connected socket, taking the host name/IP address and
 * service name/port number as arguments.
 *
 * The arguments passed to this function are used verbatim for a
 * getaddrinfo() call, and the same rules apply.
 */
struct csalt_resource_network_socket csalt_resource_network_udp_connected(
	const char *node,
	const char *service
);

/**
 * \brief Creates a bound UDP socket, taking the host name/IP address
 * and service name/port number as arguments.
 *
 * The arguments passed to this function are used verbatim for a
 * getaddrinfo() call, and the same rules apply.
 *
 * Setting node to NULL will allow binding to the wiledcard address.
 */
struct csalt_resource_network_socket csalt_resource_network_udp_bound(
	const char *node,
	const char *service
);

/**
 * \brief Creates a new UDP socket. Supports sendto and automatically
 * binds to the INADDR_ANY and a free port on recvfrom.
 */
struct csalt_resource_network_socket csalt_resource_network_udp_stateless();

/**
 * \brief Implementation of sendto for socket resources.
 *
 * You generally should not be calling this directly: use csalt_resource_sendto().
 */
ssize_t csalt_resource_socket_sendto(
	csalt_resource_network *network,
	const void *buffer,
	ssize_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
);

/**
 * \brief Implementation of recvfrom for socket resources.
 *
 * You generally should not be calling this directly: use csalt_resource_recvfrom().
 */
ssize_t csalt_resource_socket_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	ssize_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_NETWORKRESOURCES_H
