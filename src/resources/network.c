#include <csalt/networkresources.h>
#include <netdb.h>
#include <unistd.h>

static struct csalt_resource_interface csalt_addrinfo_implementation;
static struct csalt_resource_interface csalt_addrinfo_init_implementation;

struct csalt_addrinfo {
	struct csalt_resource_interface *vtable;
	const char *node;
	const char *service;
	const struct addrinfo *hints;
	struct addrinfo *result;
	int error;
};

struct csalt_addrinfo csalt_addrinfo(
	const char *node,
	const char *service,
	const struct addrinfo *hints
)
{
	struct csalt_addrinfo result = {
		.node = node,
		.service = service,
		.hints = hints,
	};
	return result;
}

void csalt_addrinfo_init(csalt_resource *resource)
{
	// naming things is hard
	struct csalt_addrinfo *infoinfo = castto(infoinfo, resource);

	// looks like this call blocks on DNS lookups?
	// TODO: non-blocking DNS lookup support... somehow
	infoinfo->error = getaddrinfo(
		infoinfo->node,
		infoinfo->service,
		infoinfo->hints,
		&infoinfo->result
	);

	if (!infoinfo->error)
		infoinfo->vtable = &csalt_addrinfo_init_implementation;
}

char csalt_addrinfo_valid(const csalt_resource *resource)
{
	struct csalt_addrinfo *infoinfo = castto(infoinfo, resource);
	return !infoinfo->error;
}

void csalt_addrinfo_deinit(csalt_resource *resource)
{
	struct csalt_addrinfo *infoinfo = castto(infoinfo, resource);
	freeaddrinfo(infoinfo->result);
	infoinfo->vtable = &csalt_addrinfo_implementation;
}

static struct csalt_resource_interface csalt_addrinfo_implementation = {
	{
		csalt_store_null_read,
		csalt_store_null_write,
		csalt_store_null_size,
		csalt_store_null_split,
	},
	csalt_addrinfo_init,
	csalt_noop_valid,
	csalt_noop_deinit,
};

static struct csalt_resource_interface csalt_addrinfo_init_implementation = {
	{
		csalt_store_null_read,
		csalt_store_null_write,
		csalt_store_null_size,
		csalt_store_null_split,
	},
	csalt_noop_init,
	csalt_addrinfo_valid,
	csalt_addrinfo_deinit,
};

ssize_t csalt_resource_sendto(
	csalt_resource_network *network,
	const void *buffer,
	size_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
) {
	return (*network)->sendto(
		network,
		buffer,
		length,
		flags,
		dest_addr,
		addr_t
	);
}

// Uninitialized sockets are set with this function, so we don't
// keep double-checking if the thing is valid
ssize_t csalt_resource_noop_sendto(
	csalt_resource_network *network,
	const void *buffer,
	size_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
) {
	return -1;
}

ssize_t csalt_resource_socket_sendto(
	csalt_resource_network *network,
	const void *buffer,
	size_t length,
	int flags,
	const struct sockaddr *dest_addr,
	socklen_t addr_t
) {
	struct csalt_resource_network_socket *sock = castto(sock, network);
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
	return (*network)->recvfrom(
		network,
		buffer,
		length,
		flags,
		src_addr,
		addrlen
	);
}

ssize_t csalt_resource_noop_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	size_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
) {
	return -1;
}

ssize_t csalt_resource_socket_recvfrom(
	csalt_resource_network *network,
	void *buffer,
	size_t length,
	int flags,
	struct sockaddr *src_addr,
	socklen_t *addrlen
) {
	struct csalt_resource_network_socket *sock = castto(sock, network);
	return recvfrom(
		sock->fd,
		buffer,
		length,
		flags,
		src_addr,
		addrlen
	);
}

struct csalt_resource_network_interface csalt_resource_network_udp_connected_implementation;
struct csalt_resource_network_interface csalt_resource_network_udp_init_connected_implementation;

// Are those names too long for you? Yeah, me too
#define udp_connected_implementation csalt_resource_network_udp_connected_implementation
#define udp_init_connected_implementation csalt_resource_network_udp_init_connected_implementation

struct csalt_resource_network_udp csalt_resource_network_udp_connected(
	const char *node,
	const char *service
)
{
	struct csalt_resource_network_udp result = {
		.parent = {
			// Using the names here because this struct has
			// a lot of members, should be easier to track this
			// way
			.vtable = &udp_connected_implementation,
			.fd = -1,
			.domain = 0,
			.type = 0,
			.protocol = 0,
			.node = node,
			.service = service,
		},
	};

	return result;
}

int use_csalt_addrinfo(csalt_resource *resource, csalt_store *store)
{
	struct csalt_addrinfo *addrinfo = castto(addrinfo, resource);
	struct csalt_resource_network_udp *udp = castto(udp, store);

	struct addrinfo *current;
	for (
		current = addrinfo->result;
		current;
		current = current->ai_next
	) {
		int sock = socket(
			current->ai_family,
			current->ai_socktype,
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
			udp->parent.fd = sock;
			break;
		}

		close(sock);
	}
}

void csalt_resource_network_udp_connected_init(csalt_resource *resource)
{
	struct csalt_resource_network_udp *udp = castto(udp, resource);

	struct addrinfo hints = {
		.ai_family = AF_INET6,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = 0,
	};
	struct csalt_addrinfo info = csalt_addrinfo(
		udp->parent.node,
		udp->parent.service,
		&hints
	);

	if (
		csalt_resource_use(
			csalt_resource(&info),
			use_csalt_addrinfo,
			csalt_store(udp)
		) != -1
	) {
		udp->vtable = &udp_init_connected_implementation;
	}
}

char csalt_resource_network_socket_valid(const csalt_resource *resource)
{
	struct csalt_resource_network_socket *sock = castto(sock, resource);

	return sock->fd >= 0;
}

void csalt_resource_network_socket_deinit(csalt_resource *resource)
{
	struct csalt_resource_network_socket *sock = castto(sock, resource);

	close(sock->fd);
	sock->fd = -1;
}

void csalt_resource_network_udp_connected_deinit(csalt_resource *resource)
{
	csalt_resource_network_socket_deinit(resource);

	struct csalt_resource_network_udp *udp = castto(udp, resource);
	udp->vtable = &udp_connected_implementation;
}

// these functions should really be shared with csalt_resource_file...
ssize_t csalt_resource_network_socket_read(
	const csalt_store *store,
	void *buffer,
	size_t amount
) {
	struct csalt_resource_network_socket *sock = castto(sock, store);
	return read(sock->fd, buffer, amount);
}

ssize_t csalt_resource_network_socket_write(
	csalt_store *store,
	const void *buffer,
	size_t amount
) {
	struct csalt_resource_network_socket *sock = castto(sock, store);
	return write(sock->fd, buffer, amount);
}

struct csalt_resource_network_interface csalt_resource_network_udp_connected_implementation = {
	{
		{
			csalt_store_null_read,
			csalt_store_null_write,
			csalt_store_null_size,
			csalt_store_null_split,
		},
		csalt_resource_network_udp_connected_init,
		csalt_noop_valid,
		csalt_noop_deinit,
	},
	csalt_resource_noop_sendto,
	csalt_resource_noop_recvfrom,
};

struct csalt_resource_network_interface csalt_resource_network_udp_init_connected_implementation = {
	{
		{
			csalt_resource_network_socket_read,
			csalt_resource_network_socket_write,
			csalt_store_null_size,
			csalt_store_null_split,
		},
		csalt_noop_init,
		csalt_resource_network_socket_valid,
		csalt_resource_network_udp_connected_deinit,
	},
	csalt_resource_socket_sendto,
	csalt_resource_socket_recvfrom,
};
