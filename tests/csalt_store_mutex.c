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

#include <csalt/platform/threads.h>

#include <errno.h>

csalt_mutex mutex;

static ssize_t stub_read(csalt_static_store *store, void *buffer, ssize_t size)
{
	(void)store;
	(void)buffer;
	if (!csalt_mutex_trylock(&mutex))
		print_error_and_exit("Mutex not locked on read");
	return size;
}

static ssize_t stub_write(csalt_static_store *store, const void *buffer, ssize_t size)
{
	(void)store;
	(void)buffer;
	if (!csalt_mutex_trylock(&mutex))
		print_error_and_exit("Mutex not locked on write");
	return size;
}

static int stub_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	(void)begin;
	(void)end;
	if (!csalt_mutex_trylock(&mutex))
		print_error_and_exit("Mutex not locked on split");
	return block(store, param);
}

struct csalt_static_store_interface impl = {
	&stub_read,
	&stub_write,
	&stub_split,
};
csalt_static_store stub = &impl;

static int split(csalt_static_store *store, void *param)
{
	(void)store;
	(void)param;
	return 0;
}

int main()
{
	struct csalt_store_mutex
		mutex_store = csalt_store_mutex((csalt_store *)&stub, &mutex);

	csalt_static_store *store = (csalt_static_store *)&mutex_store;

	csalt_store_read(store, 0, 0);
	csalt_store_write(store, 0, 0);
	csalt_store_split(store, 0, 0, split, 0);
}
