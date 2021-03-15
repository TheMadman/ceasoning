#include <csalt/stores.h>
#include <csalt/resources.h>
#include <stdlib.h>
#include <string.h>

#include "test_macros.h"

#define SPLIT_BEGIN_OFFSET 10

int *memory = 0;
char *memory_two = 0;

int split_called = 0;

int split(csalt_store *store, void *data)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	(void)data;
	split_called = 1;
	void *csalt_start = csalt_store_list_get(castto(struct csalt_store_list *, fallback), 0);
	void *start = csalt_store_memory_raw(csalt_start);

	void *first_start = &(((char *)memory)[SPLIT_BEGIN_OFFSET]);
	if (start != first_start) {
		print_error("Unexpected pointer after split");
		exit(EXIT_TEST_FAILURE);
	}

	csalt_start = csalt_store_list_get(castto(struct csalt_store_list *, fallback), 1);
	start = csalt_store_memory_raw(csalt_start);
	void *second_start = &memory_two[SPLIT_BEGIN_OFFSET];
	if (start != second_start) {
		print_error("Unexpected pointer after split");
		exit(EXIT_TEST_FAILURE);
	}
	return 0;
}



int main()
{
	struct csalt_heap csalt_memory_one = csalt_heap(20 * sizeof(int));
	struct csalt_heap csalt_memory_two = csalt_heap(20);

	memory = csalt_store_memory_raw(&csalt_memory_one.parent);
	memory_two = csalt_store_memory_raw(&csalt_memory_two.parent);

	csalt_store *stores[] = {
		castto(csalt_store *, &csalt_memory_one),
		castto(csalt_store *, &csalt_memory_two),
	};

	struct csalt_store_fallback fallback = csalt_store_fallback_array(stores);

	size_t length = csalt_store_list_length(castto(struct csalt_store_list *,&fallback));

	if (length != 2) {
		print_error("Actual length does not match expected length, actual value: %ld", length);
		return EXIT_TEST_FAILURE;
	}

	size_t size = csalt_store_size(castto(csalt_store *, &fallback));

	if (size != 20) {
		print_error("Actual size does not match expected size, actual value: %ld", size);
		return EXIT_TEST_FAILURE;
	}

	csalt_store *first = csalt_store_list_get(castto(struct csalt_store_list *, &fallback), 0);
	if (first != castto(first, &csalt_memory_one)) {
		print_error("First element pointer is unexpected value, first element: %p"
				" expected value: %p", first, &csalt_memory_one);
		return EXIT_TEST_FAILURE;
	}

	csalt_store *second = csalt_store_list_get(castto(struct csalt_store_list *, &fallback), 1);
	if (second != castto(second, &csalt_memory_two)) {
		print_error("Second element pointer is unexpected value, second element: %p"
				" expected value: %p", second, &csalt_memory_two);
		return EXIT_TEST_FAILURE;
	}

	csalt_store_split(
		castto(csalt_store *, &fallback),
		SPLIT_BEGIN_OFFSET,
		csalt_store_size(castto(csalt_store *, &fallback)),
		split,
		0
	);

	if (!split_called) {
		print_error("Split not called");
		return EXIT_TEST_FAILURE;
	}

	char data[] = "Hello, world!";
	struct csalt_memory csalt_string = csalt_store_memory_array(data);
	size_t transfer_amount = csalt_store_size(csalt_store(&csalt_string));
	struct csalt_transfer progress = csalt_transfer(transfer_amount);

	for (size_t write_size = 0; write_size < transfer_amount;) {
		write_size = csalt_store_transfer(
			&progress,
			csalt_store_list_get(castto(struct csalt_store_list *, &fallback), 1),
			csalt_store(&csalt_string),
			0
		);
		if (write_size < 0) {
			print_error("Error during attempted transfer");
			return EXIT_TEST_FAILURE;
		}
	}

	if (strcmp(memory_two, data)) {
		print_error("Expected \"%s\", got \"%s\"", data, memory_two);
		return EXIT_TEST_FAILURE;
	}

	return EXIT_SUCCESS;
}
