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

#include <csalt/stores.h>
#include <csalt/platform/threads.h>

#include "test_macros.h"

csalt_rwlock rwlock;

ssize_t stub_read(csalt_static_store *store, void *buffer, ssize_t amount)
{
	(void)store;
	(void)buffer;
	(void)amount;

	if (csalt_rwlock_trywrlock(&rwlock))
		return 0;
	csalt_rwlock_unlock(&rwlock);
	return -1;
}

ssize_t stub_write(csalt_static_store *store, const void *buffer, ssize_t amount)
{
	(void)store;
	(void)buffer;
	(void)amount;

	if (csalt_rwlock_tryrdlock(&rwlock))
		return 0;
	csalt_rwlock_unlock(&rwlock);
	return -1;
}

int stub_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	(void)begin;
	(void)end;

	return block(store, param);
}

ssize_t stub_size(csalt_store *store)
{
	(void)store;

	if (csalt_rwlock_trywrlock(&rwlock))
		return 0;
	csalt_rwlock_unlock(&rwlock);
	return -1;
}

ssize_t stub_resize(csalt_store *store, ssize_t new_size)
{
	(void)store;
	(void)new_size;

	if (csalt_rwlock_tryrdlock(&rwlock))
		return 0;
	csalt_rwlock_unlock(&rwlock);
	return -1;
}

struct csalt_dynamic_store_interface impl = {
	{
		stub_read,
		stub_write,
		stub_split,
	},
	stub_size,
	stub_resize,
};
csalt_store stub = &impl;

int tests(csalt_static_store *store, void *param);

int main()
{
	struct csalt_store_rwlock
		lock = csalt_store_rwlock(&stub, &rwlock);

	csalt_static_store *static_store = (csalt_static_store *)&lock;

	tests(static_store, 0);

	csalt_store_split(
		static_store,
		0,
		0,
		tests,
		0);
}

int tests(csalt_static_store *static_store, void *param)
{
	(void)param;

	csalt_store *store = (csalt_store *)static_store;

	if (csalt_store_read(static_store, 0, 0))
		print_error_and_exit("read lock failed");

	csalt_rwlock_rdlock(&rwlock);
	if (csalt_store_read(static_store, 0, 0))
		print_error_and_exit("read lock failed");
	csalt_rwlock_unlock(&rwlock);

	csalt_rwlock_wrlock(&rwlock);
	if (!csalt_store_read(static_store, 0, 0))
		print_error_and_exit("read lock succeeded with write lock active");
	csalt_rwlock_unlock(&rwlock);

	if (csalt_store_write(static_store, 0, 0))
		print_error_and_exit("write lock failed");

	csalt_rwlock_rdlock(&rwlock);
	if (!csalt_store_write(static_store, 0, 0))
		print_error_and_exit("write lock succeeded with read lock active");
	csalt_rwlock_unlock(&rwlock);

	csalt_rwlock_wrlock(&rwlock);
	if (!csalt_store_write(static_store, 0, 0))
		print_error_and_exit("write lock succeeded with write lock active");
	csalt_rwlock_unlock(&rwlock);

	if (csalt_store_size(store))
		print_error_and_exit("size-read lock failed");

	csalt_rwlock_rdlock(&rwlock);
	if (csalt_store_size(store))
		print_error_and_exit("size-read lock failed");
	csalt_rwlock_unlock(&rwlock);

	csalt_rwlock_wrlock(&rwlock);
	if (!csalt_store_size(store))
		print_error_and_exit("size-read lock succeeded with write lock active");
	csalt_rwlock_unlock(&rwlock);

	if (csalt_store_resize(store, 0))
		print_error_and_exit("resize-write lock failed");

	csalt_rwlock_rdlock(&rwlock);
	if (!csalt_store_resize(store, 0))
		print_error_and_exit("resize-write lock succeeded with read lock active");
	csalt_rwlock_unlock(&rwlock);

	csalt_rwlock_wrlock(&rwlock);
	if (!csalt_store_resize(store, 0))
		print_error_and_exit("resize_write lock succeeded with write lock active");
	csalt_rwlock_unlock(&rwlock);


	return 0;
}

