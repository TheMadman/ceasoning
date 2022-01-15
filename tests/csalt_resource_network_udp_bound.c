#include <csalt/networkresources.h>
#include <csalt/resources.h>
#include <csalt/util.h>
#include <string.h>

#include "test_macros.h"

#define HOST "localhost"
#define PORT "10985"

#define bound(host, port) csalt_resource_network_udp_bound(host, port)
#define connected(host, port) csalt_resource_network_udp_connected(host, port)

int use_list(csalt_store *store, void *params)
{
	struct csalt_store_pair *list = (struct csalt_store_pair *)store;
	struct csalt_resource_network_socket_initialized *bound = (void *)csalt_store_pair_list_get(list, 0);
	csalt_store *connected = csalt_store_pair_list_get(list, 1);

	const char payload[] = "Hello, World!";
	
	char recv_result[arrlength(payload)] = { 0 };
	struct sockaddr_storage addr_store = { 0 };
	struct sockaddr *addr = (struct sockaddr *)&addr_store;
	socklen_t addrlen = sizeof(store);

	csalt_store_write(connected, payload, arrlength(payload));

	ssize_t read_amount = 0;
	while (read_amount < arrlength(payload)) {
		ssize_t current_read = csalt_resource_recvfrom(
			bound,
			recv_result,
			arrlength(recv_result),
			0,
			addr,
			&addrlen
		);
		if (current_read < 0) {
			perror("recvfrom error");
			return EXIT_FAILURE;
		}
		read_amount += current_read;
	}

	if (strcmp(payload, recv_result)) {
		print_error("unexpected recv_result: %s", recv_result);
		return EXIT_FAILURE;
	}

	const char response[] = "Hello Client!";
	char sendto_result[arrlength(response)] = { 0 };

	ssize_t sendto_amount = csalt_resource_sendto(
		(csalt_resource_network *)bound,
		response,
		arrlength(response),
		0,
		addr,
		addrlen
	);

	if (sendto_amount < 0) {
		perror("sendto");
		return EXIT_FAILURE;
	}

	for (
		ssize_t amount_read = 0;
		amount_read < 1;
		amount_read = csalt_store_read(connected, sendto_result, arrlength(sendto_result))
	) {
		if (amount_read < 0) {
			perror("csalt_store_read");
			return EXIT_FAILURE;
		}
	}

	if (strcmp(response, sendto_result)) {
		print_error("unexpected sendto_result: %s", sendto_result);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_resource_network_socket bound = bound(HOST, PORT);
	struct csalt_resource_network_socket connected = connected(HOST, PORT);

	csalt_resource *array[] = {
		(csalt_resource *)&bound,
		(csalt_resource *)&connected,
	};

	struct csalt_resource_pair pairs[arrlength(array)] = { 0 };
	int error = csalt_resource_pair_list(array, pairs);
	if (error) {
		print_error("Error initializing pair list");
		return EXIT_TEST_ERROR;
	}

	return csalt_resource_use((csalt_resource *)pairs, use_list, 0);
}
