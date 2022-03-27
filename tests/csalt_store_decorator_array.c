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

int receive_split(csalt_store *, void *);

int main()
{
	{
		int numbers[5] = { 0 };
		struct csalt_memory
			csalt_numbers = csalt_memory_array(numbers);

		struct csalt_store_decorator_array
			csalt_number_array = csalt_store_decorator_array(
				(csalt_store *)&csalt_numbers,
				sizeof(int)
			);
		csalt_store *store = (csalt_store *)&csalt_number_array;

		int write_data = 1 << 16;

		ssize_t write_result = csalt_store_write(store, &write_data, 1);
		if (write_result != 1) {
			print_error("Unexpected write return value: %ld", write_result);
			return EXIT_FAILURE;
		}

		if (numbers[0] != write_data) {
			print_error("Unexpected contents in array after write: %d", numbers[0]);
			return EXIT_FAILURE;
		}
	}

	{
		int numbers[5] = { 1 << 20, 0 };
		struct csalt_memory
			csalt_numbers = csalt_memory_array(numbers);

		struct csalt_store_decorator_array
			csalt_number_array = csalt_store_decorator_array(
				(csalt_store *)&csalt_numbers,
				sizeof(int)
			);
		csalt_store *store = (csalt_store *)&csalt_number_array;

		int read_data = 0;

		ssize_t read_result = csalt_store_read(store, &read_data, 1);
		if (read_result != 1) {
			print_error("Unexpected read return value: %ld", read_result);
			return EXIT_FAILURE;
		}

		if (numbers[0] != read_data) {
			print_error("Unexpected contents in variable after read: %d", read_data);
			return EXIT_FAILURE;
		}
	}

	{
		int numbers[5] = { 0 };
		struct csalt_memory
			csalt_numbers = csalt_memory_array(numbers);

		struct csalt_store_decorator_array
			csalt_number_array = csalt_store_decorator_array(
				(csalt_store *)&csalt_numbers,
				sizeof(int)
			);
		csalt_store *store = (csalt_store *)&csalt_number_array;

		ssize_t size = csalt_store_size(store);

		if (size != 5) {
			print_error("Unexpected size returned: %ld", size);
			return EXIT_FAILURE;
		}
	}

	{
		int numbers[5] = { 0 };
		struct csalt_memory
			csalt_numbers = csalt_memory_array(numbers);

		struct csalt_store_decorator_array
			csalt_number_array = csalt_store_decorator_array(
				(csalt_store *)&csalt_numbers,
				sizeof(int)
			);
		csalt_store *store = (csalt_store *)&csalt_number_array;

		int result = csalt_store_split(
			store,
			2,
			4,
			receive_split,
			0
		);

		if (result == EXIT_FAILURE)
			return EXIT_FAILURE;

		if (numbers[2] != 1 << 20 || numbers[3] != 1 << 21) {
			print_error(
				"Unexpected values: numbers[2]=%d numbers[3]=%d",
				numbers[2],
				numbers[3]
			);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int receive_split(csalt_store *store, void *_)
{
	(void)_;

	int numbers[2] = { 1 << 20, 1 << 21 };

	ssize_t write_result = csalt_store_write(store, numbers, sizeof(numbers));

	if (write_result != sizeof(numbers)) {
		print_error("Unexpected split write value: %ld", write_result);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
