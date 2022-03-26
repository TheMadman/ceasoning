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

static ssize_t csalt_store_mutex_stub_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return amount;
}

static ssize_t csalt_store_mutex_stub_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return amount;
}

static int csalt_store_mutex_stub_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	if (!csalt_mutex_trylock(&mutex))
		return -1;
	return block(store, param);
}

struct csalt_store_interface csalt_store_mutex_stub_implementation = {
	csalt_store_mutex_stub_read,
	csalt_store_mutex_stub_write,
	0,
	csalt_store_mutex_stub_split,
};

csalt_store csalt_store_mutex_stub = &csalt_store_mutex_stub_implementation;

static int split(csalt_store *__, void *_)
{
	return 0;
}

static int test_mutex(csalt_store *store, void *_)
{
	int mutex_state = csalt_mutex_trylock(&mutex);
	csalt_mutex_unlock(&mutex);
	if (mutex_state == EINVAL) {
		print_error("Mutex was invalid after init");
		return -1;
	}


	if (csalt_store_read(store, 0, 0)) {
		print_error("Error reading store");
		return -1;
	}
	if (csalt_store_write(store, 0, 0)) {
		print_error("Error writing store");
		return -1;
	}
	if (csalt_store_split(store, 0, 0, split, 0)) {
		print_error("Error splitting store");
		return -1;
	}
	return 0;
}

int main()
{
	struct csalt_decorator_mutex decorator = csalt_decorator_mutex(&csalt_store_mutex_stub, &mutex);
	csalt_resource *resource = (csalt_resource *)&decorator;

	if (csalt_resource_use(resource, test_mutex, 0) < 0)
		return EXIT_FAILURE;

	int mutex_state = csalt_mutex_trylock(&mutex);
	csalt_mutex_unlock(&mutex);
	if (mutex_state != EINVAL) {
		print_error("Mutex was still valid after deinit");
		return EXIT_TEST_ERROR;
	}

}
