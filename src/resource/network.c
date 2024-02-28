#include "csalt/resource/network.h"

int csalt_resource_network_getaddrinfo(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	int (*callback)(const struct addrinfo *result, void *param),
	void *param
)
{
	struct addrinfo *result;
	const int error = getaddrinfo(
		node,
		service,
		hints,
		&result);

	if (error)
		return error;

	const int callback_return = callback(result, param);
	freeaddrinfo(result);
	return callback_return;
}

