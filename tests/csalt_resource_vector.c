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

#include <csalt/resources.h>

#include "test_macros.h"

int use_vector(csalt_store *store, void *_)
{
	{
		char read_buffer[1024] = { 0 };

		ssize_t amount_read = csalt_store_read(
			store,
			read_buffer,
			sizeof(read_buffer)
		);

		if (amount_read != 0) {
			print_error("Read uninitialized value");
			return EXIT_FAILURE;
		}
	}

	{
		const char write_data[] = "Hello, world!";

		char read_data[sizeof(write_data)] = { 0 };

		ssize_t write_amount = csalt_store_write(
			store,
			write_data,
			sizeof(write_data)
		);
		
		if (write_amount != sizeof(write_data)) {
			print_error(
				"Write amount was unexpected value: %ld",
				write_amount
			);
			return EXIT_FAILURE;
		}

		ssize_t read_amount = csalt_store_read(
			store,
			read_data,
			sizeof(read_data)
		);

		if (read_amount != sizeof(write_data)) {
			print_error(
				"Read amount was unexpected value: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}
	}

	{
		const char large_write[1024] = { 0 };

		char large_read[sizeof(large_write)] = { 0 };

		ssize_t write_amount = csalt_store_write(
			store,
			large_write,
			sizeof(large_write)
		);
		
		if (write_amount != sizeof(large_write)) {
			print_error(
				"Write amount was unexpected value: %ld",
				write_amount
			);
			return EXIT_FAILURE;
		}

		ssize_t read_amount = csalt_store_read(
			store,
			large_read,
			sizeof(large_read)
		);

		if (read_amount != sizeof(large_read)) {
			print_error(
				"Read amount was unexpeted value: %ld",
				read_amount
			);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_resource_vector vector = csalt_resource_vector(0);

	return csalt_resource_use(csalt_resource(&vector), use_vector, 0);
}
