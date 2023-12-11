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

const struct csalt_dynamic_store_interface impl = {
	{
		csalt_store_decorator_read,
		csalt_store_decorator_write,
		csalt_store_decorator_split,
	},
	csalt_store_decorator_size,
	csalt_store_decorator_resize,
};

typedef struct test_struct {
	struct csalt_store_decorator parent;
	struct csalt_dynamic_store_stub stub;
} test_struct_t;

// takes an out param so the pointer isn't pointing
// into invalid stack when this returns
void test_struct(struct test_struct *result, ssize_t size)
{
	*result = (test_struct_t) {
		{
			.vtable = &impl,
		},
		csalt_dynamic_store_stub(size),
	};
	result->parent.decorated = (void*)&result->stub;
}

int block(csalt_static_store *, void *);

int main()
{
	test_struct_t test = { 0 };
	test_struct(&test, 1);
	csalt_static_store *store_static = (void*)&test;

	csalt_store_write(store_static, NULL, 1);
	if (test.stub.parent.last_write != 1)
		print_error_and_exit("Write call wasn't forwarded");

	csalt_store_read(store_static, NULL, 1);
	if (test.stub.parent.last_read != 1)
		print_error_and_exit("Read call wasn't forwarded");

	csalt_store_split(store_static, 1, 3, block, NULL);

	csalt_store *store = (void*)&test;
	if (csalt_store_size(store) != 1)
		print_error_and_exit("Unexpected size");

	if (csalt_store_resize(store, 3) != 3)
		print_error_and_exit("Unexpected resize");

	return EXIT_SUCCESS;
}

int block(csalt_static_store *store, void *_)
{
	(void)_;
	struct csalt_static_store_stub *stub = (void*)store;
	if (stub->split_begin != 1)
		print_error_and_exit("Invalid begin");

	if (stub->split_end != 3)
		print_error_and_exit("Invalid end");

	return 0;
}

