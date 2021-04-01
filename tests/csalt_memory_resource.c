#include "csalt/resources.h"

#include <stdio.h>

#include "test_macros.h"

int block_called = 0;

int block(csalt_resource *_, csalt_store *__)
{
	(void)_;
	(void)__;
	block_called++;
	return 0;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap_lazy(-1);

	csalt_resource_use((csalt_resource *)&failure, block, 0);

	if (block_called) {
		print_error("Block was called on error");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap success = csalt_heap_lazy(1);

	csalt_resource_use((csalt_resource *)&success, block, 0);

	if (!block_called) {
		print_error("Block wasn't called when it should have been");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap fail_immediately = csalt_heap(-1);
	if (csalt_resource_valid(castto(csalt_resource *, &fail_immediately))) {
		print_error("Invalid malloc call returned valid check");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap succeed_immediately = csalt_heap(1);
	if (!csalt_resource_valid(castto(csalt_resource *, &succeed_immediately))) {
		print_error("Malloc call should have succeeded but returned invalid");
		return EXIT_TEST_FAILURE;
	}

	csalt_resource_deinit(castto(csalt_resource *, &succeed_immediately));

	return EXIT_SUCCESS;
}
