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
	struct csalt_resource_stub stub = csalt_resource_stub(0);
	struct csalt_resource_stub stub_2 = csalt_resource_stub(0);

	csalt_resource *resources[] = {
		csalt_resource(&stub),
		csalt_resource(&stub_2),
	};

	struct csalt_resource_pair pairs[arrlength(resources)] = { 0 };

	csalt_resource_pair_list(resources, pairs);

	csalt_store
		*store = csalt_resource_init(csalt_resource(pairs));

	struct csalt_store_pair *result = (void *)store;

	if ((void *)result->first != &stub.return_value)
		print_error_and_exit(
			"Unexpected first return value: %p -> %p",
			(void *)result->first,
			(void *)&stub.return_value
		);

	struct csalt_store_pair *second = (void *)result->second;

	if ((void *)second->first != &stub_2.return_value)
		print_error_and_exit(
			"Unexpected second return value: %p -> %p",
			(void *)second->first,
			(void *)&stub_2.return_value
		);

	csalt_resource_deinit(csalt_resource(pairs));

	if (!stub.deinit_called)
		print_error_and_exit(
			"csalt_resource_deinit() not called on stub"
		);

	if (!stub_2.deinit_called)
		print_error_and_exit(
			"csalt_resource_deinit() not called on stub_2"
		);
}
