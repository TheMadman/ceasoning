/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2024   Marcus Harrison
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

typedef struct csalt_resource_lazy lazy_t;

int split_noop(csalt_static_store *store, void *_)
{
	(void)store;
	(void)_;
	return 0;
}

int main() {
	struct csalt_resource_stub
		base_success = csalt_resource_stub(0),
		base_failure = csalt_resource_stub(1);

	lazy_t
		lazy_success = csalt_resource_lazy((void *)&base_success),
		lazy_failure = csalt_resource_lazy((void *)&base_failure);

	csalt_resource
		*success = (void*)&lazy_success,
		*failure = (void*)&lazy_failure;

	csalt_store *success_store = csalt_resource_init(success);

	if (base_success.init_called)
		print_error_and_exit("Base init() was called when it shouldn't have been");

	csalt_resource_deinit(success);

	if (base_success.deinit_called)
		print_error_and_exit("Base deinit() was called when it shouldn't have been");

	csalt_store_read((csalt_static_store *)success_store, NULL, 10);

	if (!base_success.init_called)
		print_error_and_exit("Base init() wasn't called when it should have been");

	csalt_resource_deinit(success);
	if (!base_success.deinit_called)
		print_error_and_exit("Base deinit() wasn't called when it should have been");

	csalt_store *failure_store = csalt_resource_init(failure);

	if (base_failure.init_called)
		print_error_and_exit("Base init() was called when it shouldn't have been");

	csalt_store_read((csalt_static_store *)failure_store, NULL, 10);

	if (base_failure.init_called != 1)
		print_error_and_exit("Base init() wasn't correct value: %lu", base_failure.init_called);

	csalt_store_write((csalt_static_store *)failure_store, NULL, 10);

	if (base_failure.init_called != 2)
		print_error_and_exit("Base init() wasn't correct value: %lu", base_failure.init_called);

	csalt_store_split(
		(csalt_static_store *)failure_store,
		0,
		1,
		split_noop,
		NULL);

	if (base_failure.init_called != 3)
		print_error_and_exit("Base init() wasn't correct value: %lu", base_failure.init_called);

	csalt_store_size(failure_store);

	if (base_failure.init_called != 4)
		print_error_and_exit("Base init() wasn't correct value: %lu", base_failure.init_called);

	csalt_store_resize(failure_store, 0);

	if (base_failure.init_called != 5)
		print_error_and_exit("Base init() wasn't correct value: %lu", base_failure.init_called);
}
