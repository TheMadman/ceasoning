/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <csalt/stores.h>

#include "test_macros.h"

#include <stdlib.h>

int receive_success(csalt_static_store *store, void *_)
{
	(void)_;
	struct csalt_store_fallback *fallback = (void *)store;
	struct csalt_dynamic_store_stub *first = (void *)fallback->pair.first;
	struct csalt_store_fallback *second_fallback = (void *)fallback->pair.second;
	struct csalt_dynamic_store_stub *second = (void *)second_fallback->pair.first;

	if (first->parent.split_begin != 3 || first->parent.split_end != 5) {
		print_error(
			"Unexpected first split values: %ld -> %ld",
			first->parent.split_begin,
			first->parent.split_end
		);
		exit(EXIT_FAILURE);
	}

	if (second->parent.split_begin != 3 || second->parent.split_end != 5) {
		print_error(
			"Unexpected second split values: %ld -> %ld",
			second->parent.split_begin,
			second->parent.split_end
		);
		exit(EXIT_FAILURE);
	}

	return 0;
}

int main()
{
	{
		struct csalt_dynamic_store_stub success = csalt_dynamic_store_stub(512);
		struct csalt_dynamic_store_stub success_2 = csalt_dynamic_store_stub(1024);

		csalt_store *success_store[] = {
			(csalt_store*)&success,
			(csalt_store*)&success_2,
		};

		struct csalt_store_fallback
			fallbacks[arrlength(success_store)] = { 0 };

		csalt_store_fallback_array(success_store, fallbacks);

		csalt_store *fallback = (csalt_store*)fallbacks;
		csalt_static_store *fallback_static = (csalt_static_store*)fallback;

		csalt_store_write(fallback_static, 0, 10);

		if (success.parent.last_write != 10) {
			print_error(
				"Unexpected first last_write: %ld",
				success.parent.last_write
			);
			return EXIT_FAILURE;
		}

		if (success_2.parent.last_write != 0) {
			print_error(
				"Unexpected second last_write: %ld",
				success_2.parent.last_write
			);
			return EXIT_FAILURE;
		}

		csalt_store_read(fallback_static, 0, 10);

		if (success.parent.last_read != 10) {
			print_error(
				"Unexpected first last_read: %ld",
				success.parent.last_read
			);
			return EXIT_FAILURE;
		}

		if (success_2.parent.last_read != 0) {
			print_error(
				"Unexpected second last_read: %ld",
				success_2.parent.last_read
			);
			return EXIT_FAILURE;
		}

		csalt_store_read(fallback_static, 0, 768);

		if (success.parent.last_read != 512) {
			print_error(
				"Unexpected last_read: %ld",
				success.parent.last_read
			);
			return EXIT_FAILURE;
		}

		if (success_2.parent.last_read != 256) {
			print_error(
				"Unexpected last_read:%ld",
				success_2.parent.last_read
			);
			return EXIT_FAILURE;
		}

		ssize_t size = csalt_store_size(fallback);
		if (size != success.parent.size) {
			print_error(
				"Unexpected size returned: %ld",
				size
			);
			return EXIT_FAILURE;
		}

		csalt_store_split(
			fallback_static,
			3,
			5,
			receive_success,
			0
		);
	}

	{
		struct csalt_dynamic_store_stub success = csalt_dynamic_store_stub(1024);
		struct csalt_dynamic_store_stub zero = csalt_dynamic_store_stub_zero();

		csalt_store *store[] = {
			(csalt_store*)&zero,
			(csalt_store*)&success,
		};

		struct csalt_store_fallback
			fallbacks[arrlength(store)] = { 0 };

		csalt_store_fallback_array(store, fallbacks);

		csalt_store *fallback = (void *)fallbacks;
		csalt_static_store *fallback_static = (csalt_static_store*)fallback;

		const char write_buffer[] = "Test data";

		csalt_store_write(fallback_static, write_buffer, sizeof(write_buffer));

		if (success.parent.last_write != 0) {
			print_error("Data was written to second store");
			return EXIT_FAILURE;
		}

		char read_buffer[sizeof(write_buffer)] = { 0 };

		csalt_store_read(fallback_static, read_buffer, sizeof(read_buffer));

		if (success.parent.last_read != sizeof(read_buffer)) {
			print_error(
				"Data was incorrectly read from second store: %ld",
				success.parent.last_read
			);
			return EXIT_FAILURE;
		}

		ssize_t size = csalt_store_size(fallback);

		if (size != 0) {
			print_error(
				"Unexpected size returned: %ld",
				size
			);
			return EXIT_FAILURE;
		}
	}

	{
		struct csalt_dynamic_store_stub success = csalt_dynamic_store_stub(1024);
		struct csalt_dynamic_store_stub error = csalt_dynamic_store_stub_error();

		csalt_store *store[] = {
			(csalt_store*)&error,
			(csalt_store*)&success,
		};

		struct csalt_store_fallback
			fallbacks[arrlength(store)] = { 0 };

		csalt_store_fallback_array(store, fallbacks);

		csalt_store *fallback = (void *)fallbacks;
		csalt_static_store *fallback_static = (csalt_static_store*)fallback;

		ssize_t read_amount = csalt_store_read(
			fallback_static,
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

		if (success.parent.last_read != 0) {
			print_error(
				"Unexpected read of second store: %ld",
				success.parent.last_read
			);
			return EXIT_FAILURE;
		}

		ssize_t write_amount = csalt_store_write(
			fallback_static,
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

		if (success.parent.last_write != 0) {
			print_error(
				"Unexpected write of second store: %ld",
				success.parent.last_write
			);
			return EXIT_FAILURE;
		}
	}

	{
		struct csalt_dynamic_store_stub success = csalt_dynamic_store_stub(1024);
		struct csalt_dynamic_store_stub error = csalt_dynamic_store_stub_error();

		csalt_store *store[] = {
			(csalt_store*)&success,
			(csalt_store*)&error,
		};

		struct csalt_store_fallback
			fallbacks[arrlength(store)] = { 0 };

		csalt_store_fallback_array(store, fallbacks);

		csalt_store *fallback = (csalt_store*)fallbacks;
		csalt_static_store *fallback_static = (csalt_static_store*)fallback;

		ssize_t read_amount = csalt_store_read(fallback_static, 0, 10);

		if (read_amount != 10) {
			print_error(
				"Unexpected read_amount: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}

		read_amount = csalt_store_read(fallback_static, 0, 1025);
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
