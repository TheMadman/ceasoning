#include "csaltresources.h"

#include <stdio.h>

#define EXIT_SUCCESS 0
#define EXIT_TEST_FAILURE 99
#define EXIT_TEST_SKIPPED 77

int block_called = 0;

void *block(void *_)
{
	block_called++;
	return 0;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap(-1);

	csalt_use((csalt_resource *)&failure, block);

	if (block_called) {
		return EXIT_TEST_FAILURE;
	}

	struct csalt_heap success = csalt_heap(1);

	csalt_use((csalt_resource *)&success, block);

	return block_called ? EXIT_SUCCESS : EXIT_TEST_FAILURE;
}
