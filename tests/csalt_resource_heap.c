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

int block(csalt_store *_, void *__)
{
	(void)_;
	(void)__;
	block_called++;
	return 0;
}

int main()
{
	// size -1 should cause failure
	struct csalt_heap failure = csalt_heap(-1);

	csalt_resource_use((csalt_resource *)&failure, block, 0);

	if (block_called) {
		print_error("Block was called on error");
		return EXIT_FAILURE;
	}

	struct csalt_heap success = csalt_heap(1);

	csalt_resource_use((csalt_resource *)&success, block, 0);

	if (!block_called) {
		print_error("Block wasn't called when it should have been");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
