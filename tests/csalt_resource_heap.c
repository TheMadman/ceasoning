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

#include "csalt/resources.h"

#include <stdio.h>

#include "test_macros.h"

int block_called = 0;

int block(csalt_store *store, void *__)
{
	(void)__;
	block_called++;

	csalt_static_store *s_store = (void*)store;

	{
		char c = 9;
		ssize_t write = csalt_store_write(s_store, &c, 1);
		if (write != 1)
			print_error_and_exit("Weird write: %ld", write);
	}

	{
		char c = 0;
		ssize_t read = csalt_store_read(s_store, &c, 1);
		if (read != 1)
			print_error_and_exit("Weird read: %ld", read);

		if (c != 9)
			print_error_and_exit("Unexpected read value: %d", c);
	}
	return 0;
}

int main()
{
	// size -1 should cause failure
	struct csalt_resource_heap failure = csalt_resource_heap(-1);

	csalt_resource_use((csalt_resource *)&failure, block, 0);

	if (block_called) {
		print_error("Block was called on error");
		return EXIT_FAILURE;
	}

	struct csalt_resource_heap success = csalt_resource_heap(1);

	csalt_resource_use((csalt_resource *)&success, block, 0);

	if (!block_called) {
		print_error("Block wasn't called when it should have been");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
