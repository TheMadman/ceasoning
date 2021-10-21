#include <csalt/stores.h>

#include <csalt/util.h>
#include "test_macros.h"

int receive_split(csalt_store *store, void *data);

int main()
{
	struct csalt_store_stub first = csalt_store_stub(1024);
	struct csalt_store_stub second = csalt_store_stub(512);

	csalt_store *stores[] = {
		csalt_store(&first),
		csalt_store(&second),
	};

	struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

	csalt_store_pair_list(stores, pairs);

	if (pairs[0].second != (csalt_store *)&pairs[1]) {
		print_error(
			"First pair's second didn't point to second pair: %p vs. %p",
			pairs[0].second,
			&pairs[1]
		);
		return EXIT_FAILURE;
	}

	if (pairs[1].second) {
		print_error("Last pair had non-null pointer value: %p", pairs[1].second);
		return EXIT_FAILURE;
	}

	char data[8] = { 0 };

	ssize_t written_amount = csalt_store_write(csalt_store(pairs), data, sizeof(data));

	if (first.last_write != sizeof(data)) {
		print_error("Expected data to be written to first store");
		return EXIT_FAILURE;
	}

	if (second.last_write != sizeof(data)) {
		print_error("Expected data to be written to second store");
		return EXIT_FAILURE;
	}

	if (written_amount != sizeof(data)) {
		print_error("Returned written_amount was unexpected value: %ld", written_amount);
		return EXIT_FAILURE;
	}

	ssize_t read_amount = csalt_store_read(csalt_store(pairs), data, sizeof(data));

	if (first.last_read != sizeof(data)) {
		print_error("Expected data to be read from first store");
		return EXIT_FAILURE;
	}

	if (second.last_read != 0) {
		print_error("Expected no data to be read from second store");
		return EXIT_FAILURE;
	}

	return csalt_store_split(
		csalt_store(pairs),
		5,
		10,
		receive_split,
		0
	);
}

int receive_split(csalt_store *store, void *data)
{
	struct csalt_store_pair *first_pair = (void *)store;
	struct csalt_store_pair *second_pair = (void *)first_pair->second;

	struct csalt_store_stub *first_stub = (void *)first_pair->first;
	struct csalt_store_stub *second_stub = (void *)second_pair->first;

	if (
		first_stub->split_begin != 5 ||
		first_stub->split_end != 10
	) {
		print_error(
			"First stub's bounds aren't right: %ld -> %ld",
			first_stub->split_begin,
			first_stub->split_end
		);
		return EXIT_FAILURE;
	}

	if (
		second_stub->split_begin != 5 ||
		second_stub->split_end != 10
	) {
		print_error(
			"Second stub's bounds aren't right: %ld -> %ld",
			second_stub->split_begin,
			second_stub->split_end
		);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

