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

#include <stdlib.h>

#include "test_macros.h"

int use(csalt_store *resource, void *param)
{
	struct csalt_resource_first_initialized *first = (void *)resource;

	csalt_store_write(csalt_store(first), "a", 1);

	char buffer = 0;

	struct csalt_heap **array = param;
	void
		*heap1 = array[0]->heap.memory.begin,
		*heap2 = array[1]->heap.memory.begin,
		*heap3 = array[2]->heap.memory.begin;

	if (heap1) {
		print_error("heap1 was initialized when it shouldn't have been");
		return EXIT_FAILURE;
	}

	if (!heap2) {
		print_error("heap2 should have been initialized but wasn't");
		return EXIT_FAILURE;
	}

	if (heap3) {
		print_error("heap3 was initialized when it shouldn't have been");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct csalt_heap
		// heap with size -1 should fail
		heap1 = csalt_heap(-1),
		heap2 = csalt_heap(1),
		heap3 = csalt_heap(3);

	csalt_resource *array[] = {
		csalt_resource(&heap1),
		csalt_resource(&heap2),
		csalt_resource(&heap3),
	};

	struct csalt_resource_first first = csalt_resource_first_array(array);

	return csalt_resource_use(csalt_resource(&first), use, array);
}
