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

typedef struct csalt_store_memory memory_t;

int split(csalt_static_store *, void *);

ssize_t int_offset(ssize_t index)
{
	return index * (ssize_t)sizeof(int);
}

int main()
{
	{
		int
			data = 0,
			read = 1;
		memory_t memory = csalt_store_memory_bounds(&data, &data + 1);
		csalt_static_store *const store = (void*)&memory;

		ssize_t read_amount = csalt_store_read(store, &read, sizeof(read));
		if (read_amount != sizeof(read))
			print_error_and_exit("Read returned bad size %ld", sizeof(read));

		if (read != 0)
			print_error_and_exit("Expected read value was %d, got %d", 0, read);
	}

	{
		int
			data = 0,
			write = 1;
		memory_t memory = csalt_store_memory_bounds(&data, &data + 1);
		csalt_static_store *const store = (void*)&memory;

		ssize_t write_amount = csalt_store_write(store, &write, sizeof(write));
		if (write_amount != sizeof(write))
			print_error_and_exit("Write returned bad size %ld", write_amount);

		if (data != 1)
			print_error_and_exit("Expected written value was %d, got %d", 1, write);
	}

	{
		int data[2] = { 0 };
		memory_t memory = csalt_store_memory_array(data);
		csalt_static_store *const store = (void*)&memory;

		int split_result = csalt_store_split(
			store,
			int_offset(1),
			int_offset(2),
			split,
			NULL);

		if (split_result != 0)
			print_error_and_exit(
				"Unexpected split return value %d",
				split_result);

		if (data[0] != 0)
			print_error_and_exit(
				"Write at wrong location: %d",
				data[0]);

		if (data[1] != 42)
			print_error_and_exit(
				"Wrong value at write location: %d",
				data[1]);
	}
}

int split(csalt_static_store *store, void *_)
{
	(void)_;
	const int value = 42;
	csalt_store_write(store, &value, sizeof(value));
	return 0;
}

