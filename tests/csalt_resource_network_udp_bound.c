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
	struct csalt_store_list *list = (struct csalt_store_list *)store;
	struct csalt_resource_network_udp_initialized *bound = (void *)csalt_store_list_get(list, 0);
	csalt_store *connected = csalt_store_list_get(list, 1);

	const char payload[] = "Hello, World!";
	
	char recv_result[arrlength(payload)] = { 0 };
	struct sockaddr addr = { 0 };
	socklen_t addrlen = sizeof(addr);

	csalt_store_write(connected, payload, arrlength(payload));
	csalt_resource_recvfrom(
		(csalt_resource_network *)bound,
		recv_result,
		arrlength(recv_result),
		0,
		&addr,
		&addrlen
	);

	if (strcmp(payload, recv_result)) {
		print_error("unexpected recv_result: %s", recv_result);
		return EXIT_FAILURE;
	}

	const char response[] = "Hello Client!";
	char sendto_result[arrlength(response)] = { 0 };

	csalt_resource_sendto(
		(csalt_resource_network *)bound,
		response,
		arrlength(response),
		0,
		&addr,
		addrlen
	);

	csalt_store_read(connected, sendto_result, arrlength(sendto_result));

	if (strcmp(response, sendto_result)) {
		print_error("unexpected sendto_result: %s", sendto_result);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_resource_network_udp bound = bound(HOST, PORT);
	struct csalt_resource_network_udp connected = connected(HOST, PORT);

	csalt_resource *array[] = {
		(csalt_resource *)&bound,
		(csalt_resource *)&connected,
	};

	csalt_resource_initialized *buffer[arrlength(array)] = { 0 };

	struct csalt_resource_list list = csalt_resource_list_array(array, buffer);

	return csalt_resource_use((csalt_resource *)&list, use_list, 0);
}
