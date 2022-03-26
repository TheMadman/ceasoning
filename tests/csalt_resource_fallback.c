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

int main()
{
	struct csalt_resource_stub stub = csalt_resource_stub(1024);
	struct csalt_resource_stub stub_2 = csalt_resource_stub(512);

	csalt_resource *resources[] = {
		csalt_resource(&stub),
		csalt_resource(&stub_2),
	};

	struct csalt_resource_fallback fallbacks[arrlength(resources)] = { 0 };

	csalt_resource_fallback_array(resources, fallbacks);

	csalt_resource *fallback = (void *)fallbacks;

	struct csalt_store_fallback
		*result = (void *)csalt_resource_init(fallback);

	if (result->pair.first != (void *)&stub.return_value) {
		print_error(
			"Returned first store unexpected value: %p -> %p",
			(void *)result->pair.first,
			(void *)&stub.return_value
		);
		return EXIT_FAILURE;
	}

	struct csalt_store_fallback
		*second = (void *)result->pair.second;

	if (second->pair.first != (void *)&stub_2.return_value) {
		print_error(
			"Returned second store unexpected value: %p -> %p",
			(void *)second->pair.first,
			(void *)&stub_2.return_value
		);
		return EXIT_FAILURE;
	}

	csalt_resource_deinit(fallback);

	if (!stub.deinit_called) {
		print_error("stub not cleaned up properly");
		return EXIT_FAILURE;
	}

	if (!stub_2.deinit_called) {
		print_error("stub_2 not cleaned up properly");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
