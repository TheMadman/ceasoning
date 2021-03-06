#include "csaltresources.h"

#include <stdio.h>

#include "test_macros.h"

int block_called = 0;

struct csalt_heap block(void *_)
{
	block_called++;
	return csalt_null_heap;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap(-1);

	csalt_use((csalt_resource *)&failure, block);

	if (block_called) {
		print_error("Block wasn't called");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap success = csalt_heap(1);

	csalt_use((csalt_resource *)&success, block);

	return block_called ? EXIT_SUCCESS : EXIT_TEST_FAILURE;
}
