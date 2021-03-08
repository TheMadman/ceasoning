#include "csalt/resources.h"

#include <stdio.h>

#include "test_macros.h"

int block_called = 0;

struct csalt_heap block(csalt_resource *_)
{
	block_called++;
	return csalt_null_heap;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap(-1);

	csalt_resource_use((csalt_resource *)&failure, block);

	if (block_called) {
		print_error("Block was called on error");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap success = csalt_heap(1);

	csalt_resource_use((csalt_resource *)&success, block);

	if (!block_called) {
		print_error("Block wasn't called when it should have been");
		return EXIT_TEST_FAILURE;
	}

	return EXIT_SUCCESS;
}
