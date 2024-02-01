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
	csalt_static_store_block_fn *block,
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

static int receive_split(csalt_store *_, void *__)
{
	return 0;
}

int main()
{
	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_read(csalt_store(&decorator), 0, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_write(csalt_store(&decorator), 0, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	{
		int mutex_result = csalt_mutex_init(&mutex, 0);
		if (mutex_result) {
			print_error("Error initializing mutex");
			return EXIT_TEST_ERROR;
		}

		const struct csalt_store_interface
			*mutex_stub = &csalt_store_mutex_stub_implementation;

		struct csalt_store_decorator_mutex
			decorator = csalt_store_decorator_mutex(
				csalt_store(&mutex_stub),
				&mutex
			);

		if (csalt_store_split(csalt_store(&decorator), 0, 0, receive_split, 0)) {
			print_error("Mutex was available when it should have been locked");
			return EXIT_FAILURE;
		}

		csalt_mutex_deinit(&mutex);
	}

	return EXIT_SUCCESS;
}
