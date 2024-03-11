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

#include "test_macros.h"

#include <csalt/resources.h>

INIT_IMPL(
	int,
	socket,
	ARGS(int domain, int type, int protocol),
	ARGS(domain, type, protocol))

int socket_domain = -1;
int socket_type = -1;
int socket_protocol = -1;
int socket_return = -1;
int socket_called = 0;
int socket_stub(int domain, int type, int protocol)
{
	socket_called++;
	socket_domain = domain;
	socket_type = type;
	socket_protocol = protocol;
	return socket_return;
}

INIT_IMPL(
	int,
	connect,
	ARGS(int sockfd, const struct sockaddr *addr, socklen_t addrlen),
	ARGS(sockfd, addr, addrlen))

int connect_called = 0;
int connect_stub(
	int sockfd,
	const struct sockaddr *addr,
	socklen_t addrlen
)
{
	(void)sockfd;
	(void)addr;
	(void)addrlen;
	connect_called++;
	return 0;
}

int connect_stub_two(
	int sockfd,
	const struct sockaddr *addr,
	socklen_t addrlen
)
{
	(void)sockfd;
	(void)addr;
	(void)addrlen;
	if (++connect_called < 2)
		return -1;
	return 0;
}

INIT_IMPL(
	int,
	close,
	ARGS(int fildes),
	ARGS(fildes))

int close_stub(int fildes)
{
	return 0;
}

INIT_IMPL(
	int,
	getaddrinfo,
	ARGS(
		const char *node,
		const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res
	),
	ARGS(
		node,
		service,
		hints,
		res
	))

struct addrinfo last = {
	.ai_next = NULL,
};

struct addrinfo first = {
	.ai_next = &last,
};

int getaddrinfo_stub_fail(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
)
{
	return -1;
}

int getaddrinfo_stub_one(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
)
{
	*res = &last;
	return 0;
}

int getaddrinfo_stub_two(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
)
{
	*res = &first;
	return 0;
}

INIT_IMPL(
	void,
	freeaddrinfo,
	ARGS(struct addrinfo *res),
	ARGS(res))

int freeaddrinfo_called = 0;
void freeaddrinfo_stub(struct addrinfo *res)
{
	freeaddrinfo_called++;
}

int use_network(csalt_static_store *store, void *param)
{
	(void)store;
	(void)param;
	return 0;
}

int main()
{
	SET_IMPL(socket, socket_stub);
	SET_IMPL(close, close_stub);
	SET_IMPL(freeaddrinfo, freeaddrinfo_stub);
	SET_IMPL(connect, connect_stub);

	struct csalt_resource_network_client
		client = csalt_resource_network_client(NULL, NULL, NULL);
	csalt_static_resource *resource = (csalt_static_resource *)&client;
	
	{
		SET_IMPL(getaddrinfo, getaddrinfo_stub_fail);
		int result = csalt_static_resource_use(resource, use_network, NULL);

		if (result != -1)
			print_error_and_exit("Error return value expected");

		if (socket_called != 0)
			print_error_and_exit("socket called when it shouldn't have been");

		if (connect_called != 0)
			print_error_and_exit("connect called when it shouldn't have been");
	}

	{
		SET_IMPL(getaddrinfo, getaddrinfo_stub_one);
		socket_return = 3;
		int result = csalt_static_resource_use(resource, use_network, NULL);

		if (result != 0)
			print_error_and_exit("Success return value expected");

		if (socket_called != 1)
			print_error_and_exit("socket expected to be called exactly once");

		if (connect_called != 1)
			print_error_and_exit("connect expected to be called exactly once");
	}

	{
		SET_IMPL(getaddrinfo, getaddrinfo_stub_two);
		SET_IMPL(connect, connect_stub_two);
		socket_called = 0;
		connect_called = 0;

		int result = csalt_static_resource_use(resource, use_network, NULL);

		if (result != 0)
			print_error_and_exit("Success return value expected");

		if (socket_called != 2)
			print_error_and_exit("socket expected to be called exactly twice");

		if (connect_called != 2)
			print_error_and_exit("connect expected to be called exactly twice");
	}

	SET_IMPL(close, dlsym(RTLD_NEXT, "close"));
}
