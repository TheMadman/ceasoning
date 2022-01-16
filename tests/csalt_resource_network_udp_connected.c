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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include "test_macros.h"

int use_udp(csalt_store *store, void *_)
{
	puts("client begin");
	char
		send_string[] = "Hello, world!",
		receive_buffer[sizeof(send_string)] = { 0 };

	puts("client begin write");
	ssize_t write_amount = csalt_store_write(store, send_string, sizeof(send_string));
	if (write_amount < 0) {
		perror("client error writing");
		return EXIT_FAILURE;
	}

	ssize_t read_amount = 0;
	for (
		;
		read_amount < (ssize_t)sizeof(send_string);
		read_amount = csalt_store_read(store, receive_buffer, sizeof(send_string))
	) {
		if (read_amount < 0) {
			perror("client error reading");
			return EXIT_FAILURE;
		}
	}

	printf("client received amount: %ld\n", read_amount);

	if (strncmp(send_string, receive_buffer, sizeof(receive_buffer))) {
		print_error("strings differ: \"%s\":\"%s\"", send_string, receive_buffer);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int client(const char *node, const char *service)
{
	struct csalt_resource_network_socket udp = csalt_resource_network_udp_connected(
		node,
		service
	);
	return csalt_resource_use((csalt_resource *)&udp, use_udp, 0);
}

int server(int fd)
{
	puts("server begin");
	char buffer[1024] = { 0 };
	struct sockaddr_in6 addr = { 0 };
	socklen_t addr_len = sizeof(addr);
	puts("server begin recvfrom");
	ssize_t received_length = 0;
	while (received_length == 0) {
		received_length = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
		if (received_length < 0) {
			perror("server failed to receive message");
			return EXIT_TEST_ERROR;
		} else {
			printf("server received: \"%s\"\n", buffer);
		}
	}

	if (sendto(fd, buffer, received_length, 0, (struct sockaddr *)&addr, addr_len) < 0) {
		perror("server failed to send message");
		return EXIT_TEST_ERROR;
	}

	close(fd);
	return EXIT_SUCCESS;
}

int server_manager(int sock, pid_t client_pid)
{
	pid_t server_pid;
	if ((server_pid = fork())) {
		if (server_pid < 0) {
			return EXIT_TEST_ERROR;
		}
		int wstatus;
		waitpid(client_pid, &wstatus, 0);
		if (WEXITSTATUS(wstatus)) {
			kill(server_pid, SIGTERM);
			return WEXITSTATUS(wstatus);
		}
		waitpid(server_pid, &wstatus, 0);
		return WEXITSTATUS(wstatus);
	} else {
		return server(sock);
	}
}

int main()
{
	int sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (socket < 0) {
		print_error("socket initialization failed: %s", strerror(errno));
		return EXIT_TEST_ERROR;
	}

	struct sockaddr_in6 addr = {
		AF_INET6,
		0,
		0,
		IN6ADDR_LOOPBACK_INIT,
		0,
	};
	socklen_t addr_size = sizeof(addr);

	if (bind(sock, (struct sockaddr*)&addr, addr_size)) {
		print_error("failed to bind socket: %s", strerror(errno));
		return EXIT_TEST_ERROR;
	}

	if (getsockname(sock, (struct sockaddr*)&addr, &addr_size)) {
		print_error("failed to get socket name: %s", strerror(errno));
		return EXIT_TEST_ERROR;
	}

	pid_t client_pid;
	if ((client_pid = fork())) {
		if (client_pid < 0) {
			perror("main fork()");
			return EXIT_TEST_ERROR;
		}
		return server_manager(sock, client_pid);
	} else {
		close(sock);
		char port_as_string[6] = { 0 };
		char addr_as_string[INET6_ADDRSTRLEN] = { 0 };

		const char *addr_write_attempt = inet_ntop(
			AF_INET6,
			&addr.sin6_addr,
			addr_as_string,
			INET6_ADDRSTRLEN
		);
		if (!addr_write_attempt) {
			print_error("error converting address to string");
			return EXIT_TEST_ERROR;
		} else {
			printf("addr: %s\n", addr_as_string);
		}

		int write_attempt = snprintf(
			port_as_string,
			sizeof(port_as_string),
			"%hu",
			ntohs(addr.sin6_port)
		);
		if (write_attempt < 0) {
			print_error("error converting port to string");
			return EXIT_TEST_ERROR;
		} else {
			printf("port: %s\n", port_as_string);
		}

		return client(addr_as_string, port_as_string);
	}
}
