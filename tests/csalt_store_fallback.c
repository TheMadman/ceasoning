#include <csalt/stores.h>
#include <csalt/resources.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "test_macros.h"

#define PART_ONE "Hello"
#define PART_TWO " World"
#define SPLIT_BEGIN_OFFSET (sizeof(PART_ONE) - 1)
#define SPLIT_END_OFFSET (SPLIT_BEGIN_OFFSET + sizeof(PART_TWO))

char memory[20] = { 0 };

int split(csalt_store *fallback_store, void *_)
{
	return csalt_store_write(fallback_store, PART_TWO, sizeof(PART_TWO));
}

int use_heap(csalt_store *heap, void *_)
{
	struct csalt_memory global = csalt_store_memory_array(memory);

	ssize_t write_result = csalt_store_write((csalt_store *)&global, PART_ONE, sizeof(PART_ONE));
	if (write_result < sizeof(PART_ONE)) {
		print_error(
			"Failed to set up global memory for test\n"
				"write was: %ld\n"
				"strerror: %s",
			write_result,
			strerror(errno)
		);
		return EXIT_TEST_FAILURE;
	}


	csalt_store *stores[] = {
		heap,
		(csalt_store *)&global,
	};

	struct csalt_store_fallback fallback = csalt_store_fallback_array(stores);
	csalt_store *fallback_store = (csalt_store *)&fallback;

	char buffer[20] = { 0 };
	ssize_t heap_read_result = csalt_store_read(heap, buffer, sizeof(buffer));

	if (heap_read_result) {
		print_error("Heap should not have been readable yet");
		return EXIT_TEST_FAILURE;
	}

	ssize_t fallback_read_result = csalt_store_read(fallback_store, buffer, sizeof(PART_ONE));

	if (fallback_read_result != sizeof(PART_ONE)) {
		print_error("Unexpected read result from fallback: %ld", fallback_read_result);
		return EXIT_TEST_FAILURE;
	}

	if (strncmp(buffer, PART_ONE, sizeof(PART_ONE))) {
		print_error("Unexpected buffer contents after read: %s", buffer);
		return EXIT_TEST_FAILURE;
	}

	heap_read_result = csalt_store_read(heap, buffer, sizeof(PART_ONE));
	if (heap_read_result != sizeof(PART_ONE)) {
		print_error(
			"Unexpected read result from (expected-written-to) heap: %ld",
			heap_read_result
		);
		return EXIT_TEST_FAILURE;
	}

	if (strncmp(buffer, PART_ONE, sizeof(PART_ONE))) {
		print_error("Unexpected buffer contents after read: %s", buffer);
		return EXIT_TEST_FAILURE;
	}

	ssize_t fallback_write_result = csalt_store_split(
		fallback_store,
		SPLIT_BEGIN_OFFSET,
		SPLIT_END_OFFSET,
		split,
		0
	);

	if (fallback_write_result != sizeof(PART_TWO)) {
		print_error("Unexpected write result to fallback: %ld", fallback_write_result);
		return EXIT_TEST_FAILURE;
	}

	heap_read_result = csalt_store_read(heap, buffer, sizeof(PART_ONE PART_TWO));

	if (heap_read_result != sizeof(PART_ONE PART_TWO)) {
		print_error("Unexpected read result from heap: %ld", heap_read_result);
		return EXIT_TEST_FAILURE;
	}

	if (strncmp(buffer, PART_ONE PART_TWO, sizeof(PART_ONE PART_TWO))) {
		print_error("Unexpected buffer contents: %s", buffer);
		return EXIT_TEST_FAILURE;
	}

	if (strncmp(memory, PART_ONE, sizeof(PART_ONE))) {
		print_error("memory string changed when it shouldn't have: %s", memory);
		return EXIT_TEST_FAILURE;
	}

	struct csalt_transfer transfers[2] = { 0 };
	csalt_store_fallback_flush(&fallback, transfers);

	if (strncmp(memory, PART_ONE PART_TWO, sizeof(PART_ONE PART_TWO))) {
		print_error("memory string wasn't updated: %s", memory);
		return EXIT_TEST_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_heap heap = csalt_heap(20);

	return csalt_resource_use((csalt_resource *)&heap, use_heap, 0);
}
