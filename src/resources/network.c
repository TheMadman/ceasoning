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

#include <csalt/networkresources.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

static struct csalt_resource_interface csalt_addrinfo_implementation;
static struct csalt_store_interface csalt_addrinfo_init_implementation;

struct csalt_addrinfo_initialized {
	struct csalt_store_interface *vtable;
	const char *node;
	const char *service;
	const struct addrinfo *hints;
	struct addrinfo *result;
};

struct csalt_addrinfo {
	struct csalt_resource_interface *vtable;
	struct csalt_addrinfo_initialized addrinfo;
};

struct csalt_addrinfo csalt_addrinfo(
	const char *node,
	const char *service,
	const struct addrinfo *hints
)
{
	struct csalt_addrinfo result = {
		&csalt_addrinfo_implementation,
		{
			.vtable = &csalt_addrinfo_init_implementation,
			.node = node,
			.service = service,
			.hints = hints,
		},
	};
	return result;
}

csalt_store *csalt_addrinfo_init(csalt_resource *resource)
{
	// naming things is hard
	struct csalt_addrinfo *infoinfo = castto(infoinfo, resource);

	// looks like this call blocks on DNS lookups?
	// TODO: non-blocking DNS lookup support... somehow
	int error = getaddrinfo(
		infoinfo->addrinfo.node,
		infoinfo->addrinfo.service,
		infoinfo->addrinfo.hints,
		&infoinfo->addrinfo.result
	);

	if (!error)
		return (csalt_store *)&infoinfo->addrinfo;
	return 0;
}

void csalt_addrinfo_deinit(csalt_resource *resource)
{
	struct csalt_addrinfo *infoinfo = castto(infoinfo, resource);
	freeaddrinfo(infoinfo->addrinfo.result);
	infoinfo->addrinfo.result = 0;
}

static struct csalt_resource_interface csalt_addrinfo_implementation = {
	csalt_addrinfo_init,
	csalt_addrinfo_deinit,
};

/*
 * doesn't implement store_read/write/... because addrinfo is a
 * linked list
 *
 * seriously, who still uses linked lists...?
 *
 * Could do a read function that returns a pointer but w/e
 */
static struct csalt_store_interface csalt_addrinfo_init_implementation = {
	csalt_store_null_read,
	csalt_store_null_write,
	csalt_store_null_size,
	csalt_store_null_split,
};

/*
 * Convenience for creating/using a addrinfo resource from a udp
 */
static csalt_store *addrinfo_from_network(struct csalt_resource_network_socket *udp, csalt_store_block_fn *use)
{
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = 0,
	};
	struct csalt_addrinfo info = csalt_addrinfo(
		udp->node,
		udp->service,
		&hints
	);

	if (
		csalt_resource_use(
			csalt_resource(&info),
			use,
			csalt_store(&udp->udp)
		) != -1
	) {
		return (csalt_store *)&udp->udp;
	}

	return 0;
}

ssize_t csalt_resource_sendto(
	csalt_resource_network *network,
	const void *buffer,
	size_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
) {
	return network->network_vtable->sendto(
		network,
		buffer,
		length,
		flags,
		dest_addr,
		addr_t
	);
}

ssize_t csalt_resource_socket_sendto(
	csalt_resource_network *network,
	const void *buffer,
	size_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
) {
	struct csalt_resource_network_socket_initialized *sock = castto(sock, network);
	return sendto(
		sock->fd,
		buffer,
		length,
		flags,
		dest_addr,
		addr_t
	);
}

ssize_t csalt_resource_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	size_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
) {
	return network->network_vtable->recvfrom(
		network,
		buffer,
		length,
		flags,
		src_addr,
		addrlen
	);
}

ssize_t csalt_resource_socket_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	size_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
) {
	struct csalt_resource_network_socket_initialized *sock = castto(sock, network);
	ssize_t result = recvfrom(
		sock->fd,
		buffer,
		length,
		flags,
		src_addr,
		addrlen
	);
	if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	if (result == 0) {
		errno = ECONNRESET;
		return -1;
	}
	return result;
}

struct csalt_resource_interface udp_connected_implementation;
struct csalt_store_interface udp_init_connected_implementation;
struct csalt_resource_network_initialized_interface udp_init_connected_net_implementation;

struct csalt_resource_network_socket csalt_resource_network_udp_connected(
	const char *node,
	const char *service
)
{
	struct csalt_resource_network_socket result = {
		.vtable = &udp_connected_implementation,
		.udp = {
			// Using the names here because this struct has
			// a lot of members, should be easier to track this
			// way
			.vtable = &udp_init_connected_implementation,
			.network_vtable = &udp_init_connected_net_implementation,
			.fd = -1,
			.domain = 0,
			.type = 0,
			.protocol = 0,
		},
		.node = node,
		.service = service,
	};

	return result;
}

int use_csalt_addrinfo_connected(csalt_store *resource, void *store)
{
	struct csalt_addrinfo_initialized *addrinfo = (void *)resource;
	struct csalt_resource_network_socket_initialized *udp = store;

	struct addrinfo *current;
	for (
		current = addrinfo->result;
		current;
		current = current->ai_next
	) {
		int sock = socket(
			current->ai_family,
			current->ai_socktype | SOCK_NONBLOCK,
			current->ai_protocol
		);
		if (sock == -1)
			continue;

		int connect_return = connect(
			sock,
			current->ai_addr,
			current->ai_addrlen
		);
		if (connect_return != -1) {
			udp->fd = sock;
			return 0;
		}

		close(sock);
	}
	return -1;
}

csalt_store *csalt_resource_network_udp_connected_init(csalt_resource *resource)
{
	struct csalt_resource_network_socket *udp = (void *)resource;

	return addrinfo_from_network(udp, use_csalt_addrinfo_connected);
}

void csalt_resource_network_socket_deinit(csalt_resource *resource)
{
	struct csalt_resource_network_socket *sock = (void *)resource;

	close(sock->udp.fd);
	sock->udp.fd = -1;
}

// these functions should really be shared with csalt_resource_file...
ssize_t csalt_resource_network_socket_read(
	csalt_store *store,
	void *buffer,
	size_t amount
) {
	struct csalt_resource_network_socket_initialized *sock = castto(sock, store);
	ssize_t result = read(sock->fd, buffer, amount);
	if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	if (result == 0) {
		errno = ECONNRESET;
		return -1;
	}
	return result;
}

ssize_t csalt_resource_network_socket_write(
	csalt_store *store,
	const void *buffer,
	size_t amount
) {
	struct csalt_resource_network_socket_initialized *sock = castto(sock, store);
	return write(sock->fd, buffer, amount);
}

// maybe THIS should be the null_split operation...
int csalt_resource_network_socket_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	(void)begin;
	(void)end;
	return block(store, data);
}

struct csalt_resource_interface udp_connected_implementation = {
	csalt_resource_network_udp_connected_init,
	csalt_resource_network_socket_deinit,
};

struct csalt_store_interface udp_init_connected_implementation = {
	csalt_resource_network_socket_read,
	csalt_resource_network_socket_write,
	csalt_store_null_size,
	csalt_resource_network_socket_split,
};

struct csalt_resource_network_initialized_interface csalt_resource_network_udp_init_connected_implementation = {
	csalt_resource_socket_sendto,
	csalt_resource_socket_recvfrom,
};

struct csalt_resource_interface udp_bound_implementation;
struct csalt_store_interface udp_bound_init_implementation;
struct csalt_resource_network_initialized_interface udp_bound_init_net_implementation;

struct csalt_resource_network_socket csalt_resource_network_udp_bound(
	const char *node,
	const char *service
)
{
	struct csalt_resource_network_socket result = {
		.vtable = &udp_bound_implementation,
		.node = node,
		.service = service,
		.udp = {
			.vtable = &udp_bound_init_implementation,
			.network_vtable = &udp_bound_init_net_implementation,
			.fd = -1,
			.domain = 0,
			.type = 0,
			.protocol = 0,
		},
	};

	return result;
}

int use_csalt_addrinfo_bound(csalt_store *resource, void *store)
{
	struct csalt_addrinfo_initialized *addrinfo = castto(addrinfo, resource);
	struct csalt_resource_network_socket_initialized *udp = castto(udp, store);

	struct addrinfo *current;
	for (
		current = addrinfo->result;
		current;
		current = current->ai_next
	) {
		int sock = socket(
			current->ai_family,
			current->ai_socktype | SOCK_NONBLOCK,
			current->ai_protocol
		);
		if (sock == -1)
			continue;

		int bind_return = bind(
			sock,
			current->ai_addr,
			current->ai_addrlen
		);
		if (bind_return != -1) {
			udp->fd = sock;
			return 0;
		}

		close(sock);
	}
	return -1;
}


csalt_store *csalt_resource_network_udp_bound_init(csalt_resource *resource)
{
	struct csalt_resource_network_socket *udp = (void *)resource;

	return addrinfo_from_network(udp, use_csalt_addrinfo_bound);
}

struct csalt_resource_interface udp_bound_implementation = {
	csalt_resource_network_udp_bound_init,
	csalt_resource_network_socket_deinit,
};

struct csalt_store_interface udp_bound_init_implementation = {
	csalt_resource_network_socket_read,
	csalt_store_null_write,
	csalt_store_null_size,
	csalt_resource_network_socket_split,
};

struct csalt_resource_network_initialized_interface udp_bound_init_net_implementation = {
	csalt_resource_socket_sendto,
	csalt_resource_socket_recvfrom,
};

struct csalt_resource_interface udp_stateless_implementation;
struct csalt_store_interface udp_stateless_init_implementation;
struct csalt_resource_network_initialized_interface udp_stateless_init_net_implementation;

struct csalt_resource_network_socket csalt_resource_network_udp_stateless()
{
	struct csalt_resource_network_socket result = {
		.vtable = &udp_stateless_implementation,
		.node = 0,
		.service = 0,
		.udp = {
			.vtable = &udp_stateless_init_implementation,
			.network_vtable = &udp_stateless_init_net_implementation,
			.fd = -1,
			.domain = 0,
			.type = 0,
			.protocol = 0,
		},
	};

	return result;
}

csalt_store *csalt_resource_network_udp_stateless_init(csalt_resource *resource)
{
	struct csalt_resource_network_socket *udp = (void *)resource;

	udp->udp.fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (udp->udp.fd >= 0)
		return (csalt_store *)&udp->udp;

	return 0;
}

struct csalt_resource_interface udp_stateless_implementation = {
	csalt_resource_network_udp_stateless_init,
	csalt_resource_network_socket_deinit,
};

struct csalt_store_interface udp_stateless_init_implementation = {
	csalt_store_null_read,
	csalt_store_null_write,
	csalt_store_null_size,
	csalt_resource_network_socket_split,
};

struct csalt_resource_network_initialized_interface udp_stateless_init_net_implementation = {
	csalt_resource_socket_sendto,
	csalt_resource_socket_recvfrom,
};

