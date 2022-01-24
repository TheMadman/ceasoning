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

#include <csalt/resources.h>
#include <csalt/networkresources.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "test_macros.h"

int use_udp(csalt_store *store, void *_)
{
	csalt_resource_network *udp = (void *)store;

	int sock = socket(AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (sock < 0) {
		perror("error creating socket");
		return EXIT_FAILURE;
	}

	struct sockaddr_in6 addr = { AF_INET6 };
	addr.sin6_addr = in6addr_loopback;
	socklen_t len = sizeof(addr);

	if (bind(sock, (struct sockaddr *)&addr, len) < 0) {
		perror("error binding socket");
		close(sock);
		return EXIT_FAILURE;
	}

	if (getsockname(sock, (struct sockaddr *)&addr, &len)) {
		perror("error getting socket name");
		close(sock);
		return EXIT_FAILURE;
	}

	char buffer[] = "Hello, world!";

	ssize_t sent = csalt_resource_sendto(
		udp,
		buffer,
		sizeof(buffer),
		0,
		(struct sockaddr *)&addr,
		len
	);
	if (sent < 0) {
		perror("error sending message");
		close(sock);
		return EXIT_FAILURE;
	}

	if (sent < sizeof(buffer)) {
		print_error("unexpected buffer amount: %ld", sent);
		close(sock);
		return EXIT_FAILURE;
	}

	char receive[sizeof(buffer)] = { 0 };
	size_t received = read(sock, receive, sizeof(receive));

	if (strcmp(receive, buffer)) {
		print_error("unexpected receive: \"%s\" (length %ld)", receive, received);
		close(sock);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_resource_network_socket udp = csalt_resource_network_udp_stateless();
	return csalt_resource_use((csalt_resource *)&udp, use_udp, 0);
}

