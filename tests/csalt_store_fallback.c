#include <csalt/stores.h>
#include <csalt/resources.h>
#include <stdlib.h>
#include <string.h>

#include "test_macros.h"

#define SPLIT_BEGIN_OFFSET 10

int *memory = 0;
char *memory_two = 0;

int split_called = 0;

char data[] = "Hello, world!";
char moredata[] = " How are you?";

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

int split_for_moredata(csalt_store *store, void *data)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	size_t written = csalt_store_write(csalt_store(fallback), moredata, sizeof(moredata));

	if (written != sizeof(moredata)) {
		print_error("Unexpected write amount, expected: %ld actual: %ld",
			sizeof(moredata),
			written
		);
		exit(EXIT_TEST_FAILURE);
	}
	return 0;
}

int main()
{
	struct csalt_heap csalt_memory_one = csalt_heap(20 * sizeof(int));
	struct csalt_heap csalt_memory_two = csalt_heap(30);

	memory = csalt_store_memory_raw(&csalt_memory_one.parent);
	memory_two = csalt_store_memory_raw(&csalt_memory_two.parent);

	csalt_store *stores[] = {
		csalt_store(&csalt_memory_one),
		csalt_store(&csalt_memory_two),
	};

	struct csalt_store_fallback fallback = csalt_store_fallback_array(stores);

	size_t length = csalt_store_list_length(castto(struct csalt_store_list *,&fallback));

	if (length != 2) {
		print_error("Actual length does not match expected length, actual value: %ld", length);
		return EXIT_TEST_FAILURE;
	}

	size_t size = csalt_store_size(castto(csalt_store *, &fallback));

	if (size != 30) {
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

	size_t data_size = sizeof(data);
	struct csalt_memory csalt_string = csalt_store_memory_array(data);
	struct csalt_transfer progress = csalt_transfer(data_size);

	for (size_t write_size = 0; write_size < data_size;) {
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

	char read_buffer[sizeof(data)] = { 0 };

	csalt_store_read(csalt_store(&fallback), read_buffer, sizeof(data));

	if (strcmp(read_buffer, data)) {
		print_error("Expected \"%s\", got \"%s\"", data, read_buffer);
		return EXIT_TEST_FAILURE;
	}

	if (strcmp((char *)memory, data)) {
		print_error("Expected \"%s\", got \"%s\"", data, read_buffer);
		return EXIT_TEST_FAILURE;
	}

	// we want to overwrite the null terminator
	csalt_store_split(
		csalt_store(&fallback),
		data_size - 1,
		data_size + sizeof(moredata) - 1,
		split_for_moredata,
		moredata
	);
	const char *expected = "Hello, world! How are you?";
	if (strcmp((char *)memory, expected)) {
		print_error("Unexpected string contents, expected: %s actual: %s",
			expected,
			(char *)memory
		);
		return EXIT_TEST_FAILURE;
	}

	if (!strcmp(memory_two, expected)) {
		print_error("Data was written to second store sooner than expected");
		return EXIT_TEST_FAILURE;
	}

	struct csalt_transfer transfers[arrlength(stores)] = { 0 };
	struct csalt_transfer total_progress = csalt_store_fallback_flush(&fallback, transfers);
	if (strcmp(memory_two, expected)) {
		print_error("Flush didn't write the data out to the second store");
		return EXIT_TEST_FAILURE;
	}

	csalt_resource_deinit(csalt_resource(&csalt_memory_one));
	csalt_resource_deinit(csalt_resource(&csalt_memory_two));

	return EXIT_SUCCESS;
}