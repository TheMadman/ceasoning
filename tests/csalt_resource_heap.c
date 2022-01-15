#include "csalt/resources.h"

#include <stdio.h>

#include "test_macros.h"

int block_called = 0;

int block(csalt_store *_, void *__)
{
	(void)_;
	(void)__;
	block_called++;
	return 0;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap(-1);

	csalt_resource_use((csalt_resource *)&failure, block, 0);

	if (block_called) {
		print_error("Block was called on error");
		return EXIT_FAILURE;
	}

	struct csalt_heap success = csalt_heap(1);

	csalt_resource_use((csalt_resource *)&success, block, 0);

	if (!block_called) {
		print_error("Block wasn't called when it should have been");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
