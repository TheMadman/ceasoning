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

int use_called = 0;

int use(csalt_store *resource, void *out)
{
	use_called = 1;
	return 0;
}

int main()
{
	struct csalt_heap
		heap1 = csalt_heap(1),
		heap2 = csalt_heap(-1);

	csalt_resource *array[] = {
		csalt_resource(&heap1),
		csalt_resource(&heap2),
	};

	csalt_store *buffer[2] = { 0 };
	struct csalt_resource_list list = csalt_resource_list_array(array, buffer);

	csalt_resource_use(csalt_resource(&list), use, 0);
	if (use_called) {
		print_error("use() was called when it shouldn't have been");
		return EXIT_FAILURE;
	}

	struct csalt_heap
		heap3 = csalt_heap(4),
		heap4 = csalt_heap(8);

	array[0] = csalt_resource(&heap3);
	array[1] = csalt_resource(&heap4);

	// list = csalt_resource_list_array(array, buffer);

	csalt_resource_use(csalt_resource(&list), use, 0);
	if (!use_called) {
		print_error("use() was not called when it should have been");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

