#include <csalt/stores.h>

#include "test_macros.h"

#include <stdlib.h>

int receive_success(csalt_store *store, void *_)
{
	(void)_;
	struct csalt_store_fallback *fallback = (void *)store;
	struct csalt_store_stub *first = (void *)fallback->pair.first;
	struct csalt_store_fallback *second_fallback = (void *)fallback->pair.second;
	struct csalt_store_stub *second = (void *)second_fallback->pair.first;

	if (first->split_begin != 3 || first->split_end != 5) {
		print_error(
			"Unexpected first split values: %ld -> %ld",
			first->split_begin,
			first->split_end
		);
		exit(EXIT_FAILURE);
	}

	if (second->split_begin != 3 || second->split_end != 5) {
		print_error(
			"Unexpected second split values: %ld -> %ld",
			second->split_begin,
			second->split_end
		);
		exit(EXIT_FAILURE);
	}

	return 0;
}

int main()
{
	{
		struct csalt_store_stub success = csalt_store_stub(512);
		struct csalt_store_stub success_2 = csalt_store_stub(1024);

		csalt_store *success_stores[] = {
			csalt_store(&success),
			csalt_store(&success_2),
		};

		struct csalt_store_fallback
			fallbacks[arrlength(success_stores)] = { 0 };

		csalt_store_fallback_array(success_stores, fallbacks);

		csalt_store *fallback = csalt_store(fallbacks);

		csalt_store_write(fallback, 0, 10);

		if (success.last_write != 10) {
			print_error(
				"Unexpected first last_write: %ld",
				success.last_write
			);
			return EXIT_FAILURE;
		}

		if (success_2.last_write != 0) {
			print_error(
				"Unexpected second last_write: %ld",
				success_2.last_write
			);
			return EXIT_FAILURE;
		}

		csalt_store_read(fallback, 0, 10);

		if (success.last_read != 10) {
			print_error(
				"Unexpected first last_read: %ld",
				success.last_read
			);
			return EXIT_FAILURE;
		}

		if (success_2.last_read != 0) {
			print_error(
				"Unexpected second last_read: %ld",
				success_2.last_read
			);
			return EXIT_FAILURE;
		}

		csalt_store_read(fallback, 0, 768);

		if (success.last_read != 512) {
			print_error(
				"Unexpected last_read: %ld",
				success.last_read
			);
			return EXIT_FAILURE;
		}

		if (success_2.last_read != 256) {
			print_error(
				"Unexpected last_read:%ld",
				success_2.last_read
			);
			return EXIT_FAILURE;
		}

		size_t size = csalt_store_size(fallback);
		if (size != success.size) {
			print_error(
				"Unexpected size returned: %ld",
				size
			);
			return EXIT_FAILURE;
		}

		csalt_store_split(
			fallback,
			3,
			5,
			receive_success,
			0
		);
	}

	{
		struct csalt_store_stub success = csalt_store_stub(1024);
		struct csalt_store_stub zero = csalt_store_stub_zero();

		csalt_store *stores[] = {
			csalt_store(&zero),
			csalt_store(&success),
		};

		struct csalt_store_fallback
			fallbacks[arrlength(stores)] = { 0 };

		csalt_store_fallback_array(stores, fallbacks);

		csalt_store *fallback = (void *)fallbacks;

		const char write_buffer[] = "Test data";

		csalt_store_write(fallback, write_buffer, sizeof(write_buffer));

		if (success.last_write != 0) {
			print_error("Data was written to second store");
			return EXIT_FAILURE;
		}

		char read_buffer[sizeof(write_buffer)] = { 0 };

		csalt_store_read(fallback, read_buffer, sizeof(read_buffer));

		if (success.last_read != sizeof(read_buffer)) {
			print_error(
				"Data was incorrectly read from second store: %ld",
				success.last_read
			);
			return EXIT_FAILURE;
		}

		size_t size = csalt_store_size(fallback);

		if (size != 0) {
			print_error(
				"Unexpected size returned: %ld",
				size
			);
			return EXIT_FAILURE;
		}
	}

	{
		struct csalt_store_stub success = csalt_store_stub(1024);
		struct csalt_store_stub error = csalt_store_stub_error();

		csalt_store *stores[] = {
			csalt_store(&error),
			csalt_store(&success),
		};

		struct csalt_store_fallback
			fallbacks[arrlength(stores)] = { 0 };

		csalt_store_fallback_array(stores, fallbacks);

		csalt_store *fallback = (void *)fallbacks;

		ssize_t read_amount = csalt_store_read(
			fallback,
			0,
			10
		);

		if (read_amount != -1) {
			print_error(
				"Unexpected error read: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}

		if (success.last_read != 0) {
			print_error(
				"Unexpected read of second store: %ld",
				success.last_read
			);
			return EXIT_FAILURE;
		}

		ssize_t write_amount = csalt_store_write(
			fallback,
			0,
			10
		);

		if (write_amount != -1) {
			print_error(
				"Unexpected error written: %ld",
				write_amount
			);
			return EXIT_FAILURE;
		}

		if (success.last_write != 0) {
			print_error(
				"Unexpected write of second store: %ld",
				success.last_write
			);
			return EXIT_FAILURE;
		}
	}

	{
		struct csalt_store_stub success = csalt_store_stub(1024);
		struct csalt_store_stub error = csalt_store_stub_error();

		csalt_store *stores[] = {
			csalt_store(&success),
			csalt_store(&error),
		};

		struct csalt_store_fallback
			fallbacks[arrlength(stores)] = { 0 };

		csalt_store_fallback_array(stores, fallbacks);

		csalt_store *fallback = csalt_store(fallbacks);

		ssize_t read_amount = csalt_store_read(fallback, 0, 10);

		if (read_amount != 10) {
			print_error(
				"Unexpected read_amount: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}

		read_amount = csalt_store_read(fallback, 0, 1025);
		if (read_amount != -1) {
			print_error(
				"Unexpected read_amount: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
