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

int main()
{
	{
		struct csalt_resource_stub
			first = csalt_resource_stub(0),
			second = csalt_resource_stub(0);

		csalt_resource *resources[] = {
			(csalt_resource *)&first,
			(csalt_resource *)&second,
		};

		struct csalt_resource_first
			first_list[arrlength(resources)] = { 0 };

		csalt_resource_first_list(resources, first_list);

		csalt_store *result = csalt_resource_init(
			(csalt_resource *)first_list);

		if (result != (csalt_store *)&first.return_value)
			print_error_and_exit("Result was unexpected value, expected: %p actual: %p", &first.return_value, result);
	}

	{
		struct csalt_resource_stub
			first = csalt_resource_stub(1),
			second = csalt_resource_stub(0);

		csalt_resource *resources[] = {
			(csalt_resource *)&first,
			(csalt_resource *)&second,
		};

		struct csalt_resource_first
			first_list[arrlength(resources)] = { 0 };

		csalt_resource_first_list(resources, first_list);

		csalt_store *result = csalt_resource_init(
			(csalt_resource *)first_list);

		if (result != (csalt_store *)&second.return_value)
			print_error_and_exit("Result was unexpected value, expected: %p actual: %p", &second.return_value, result);
	}

	{
		struct csalt_resource_stub
			first = csalt_resource_stub(1),
			second = csalt_resource_stub(1);

		csalt_resource *resources[] = {
			(csalt_resource *)&first,
			(csalt_resource *)&second,
		};

		struct csalt_resource_first
			first_list[arrlength(resources)] = { 0 };

		csalt_resource_first_list(resources, first_list);

		csalt_store *result = csalt_resource_init(
			(csalt_resource *)first_list);

		if (result != NULL)
			print_error_and_exit("Result was unexpected value, expected: %p actual: %p", NULL, result);
	}
}

