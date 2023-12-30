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

static int receive_split(csalt_static_store*, void*);

int main()
{
	int buffer[8] = { 0 };

	struct csalt_store_memory
		memory = csalt_store_memory_array(buffer);

	struct csalt_store_array
		array = csalt_store_array((csalt_store*)&memory, sizeof(int));

	csalt_store *store = (csalt_store*)&array;
	csalt_static_store *static_store = (csalt_static_store*)store;

	{
		static const int first_write[2] = { 1, 2 };

		const ssize_t result = csalt_store_write(static_store, first_write, arrlength(first_write));

		if (result != 2)
			print_error_and_exit("Unexpected write result: %ld", result);

		if (buffer[0] != 1)
			print_error_and_exit("Unexpected value in zeroth location: %d", buffer[0]);

		if (buffer[1] != 2)
			print_error_and_exit("Unexpected value in first location: %d", buffer[1]);
	}

	{
		int first_read = 0;

		const ssize_t result = csalt_store_read(static_store, &first_read, 1);

		if (result != 1)
			print_error_and_exit("Unexpected read result: %ld", result);

		if (first_read != 1)
			print_error_and_exit("Unexpected value read: %d", first_read);
	}

	{
		const int result = csalt_store_split(
			static_store,
			2,
			8,
			receive_split,
			NULL);

		if (result != 0)
			print_error_and_exit("Unexpected split value: %d", result);

		if (buffer[2] != 3)
			print_error_and_exit("Unexpected value in second position: %d", buffer[2]);

		if (buffer[3] != 4)
			print_error_and_exit("Unexpected value in third position: %d", buffer[3]);
	}
}

static int receive_split(csalt_static_store *store, void *param)
{
	(void)param;

	static const int write[2] = { 3, 4 };

	csalt_store_write(store, write, sizeof(write));

	return 0;
}
