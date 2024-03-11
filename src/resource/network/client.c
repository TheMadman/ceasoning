#include "csalt/resource/network/client.h"

#include <unistd.h>

#include "csalt/resource/network.h" // getaddrinfo interface

typedef struct csalt_resource_network_client network_t;
typedef struct csalt_store_network_client store_t;

static const struct csalt_static_resource_interface impl = {
	csalt_resource_network_client_init,
	csalt_resource_network_client_deinit,
};

static const struct csalt_static_store_interface store_impl = {
	csalt_store_network_client_read,
	csalt_store_network_client_write,
	csalt_store_network_client_split,
};

struct csalt_resource_network_client csalt_resource_network_client(
	const char *node,
	const char *service,
	const struct addrinfo *hints
)
{
	return (network_t) {
		&impl,
		node,
		service,
		hints,

		{
			.vtable = &store_impl,
		},
	};
}

static int init_store(const struct addrinfo *result, void *param)
{
	store_t *store = param;
	int socket_fd = -1;

	for (; result; result = result->ai_next) {
		socket_fd = socket(
			result->ai_family,
			result->ai_socktype,
			result->ai_protocol);
		if (socket_fd == -1)
			continue;

		if (connect(socket_fd, result->ai_addr, result->ai_addrlen) != -1)
			break;

		close(socket_fd);
	}

	if (result == NULL)
		return -1;

	store->fd = socket_fd;
	return 0;
}

csalt_static_store *csalt_resource_network_client_init(
	csalt_static_resource *resource
)
{
	network_t *network = (network_t *)resource;

	const int error = csalt_resource_network_getaddrinfo(
		network->node,
		network->service,
		network->hints,
		init_store,
		&network->result);

	if (error)
		return NULL;

	return (csalt_static_store *)&network->result;
}

void csalt_resource_network_client_deinit(csalt_resource *resource)
{
	network_t *network = (network_t *)resource;
	close(network->result.fd);
	network->result.fd = -1;
}

ssize_t csalt_store_network_client_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
)
{
	store_t *network = (store_t *)store;
	return read(network->fd, buffer, (size_t)size);
}

ssize_t csalt_store_network_client_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
)
{
	store_t *network = (store_t *)store;
	return write(network->fd, buffer, (size_t)size);
}

int csalt_store_network_client_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	(void)begin;
	(void)end;
	return block(store, param);
}


