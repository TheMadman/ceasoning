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

#include "csalt/stores.h"
#include <stdio.h>

#include "test_macros.h"

ssize_t read_called = 0;
ssize_t write_called = 0;
ssize_t split_begin = 0;
ssize_t split_end = 100;

int split_block(csalt_static_store *store, void *_);

struct test_struct {
	struct csalt_static_store_interface *implementation;
	ssize_t begin;
	ssize_t end;
};

ssize_t test_read(csalt_static_store *store, void *buffer, ssize_t size)
{
	(void)store;
	(void)buffer;
	(void)size;
	read_called++;
	return 0;
}

ssize_t test_write(csalt_static_store *store, const void *buffer, ssize_t size)
{
	(void)store;
	(void)buffer;
	(void)size;
	write_called++;
	return 0;
}

ssize_t test_size(csalt_static_store *store)
{
	const struct test_struct *data = (struct test_struct *)store;
	return data->end - data->begin;
}

int test_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_fn *block,
	void *_
)
{
	(void)_;
	struct test_struct *data = (struct test_struct *)store;
	struct test_struct result = {
		data->implementation,
		data->begin + begin,
		data->begin + end
	};
	return block((csalt_static_store *)&result, data);
}

struct csalt_static_store_interface test_implementation = {
	test_read,
	test_write,
	test_split,
};

int split_block(csalt_static_store *store, void *_)
{
	(void)_;
	struct test_struct *impl = (struct test_struct *)store;
	split_begin = impl->begin;
	split_end = impl->end;
	return 0;
}

int main()
{
	struct test_struct data = {
		&test_implementation,
		split_begin,
		split_end,
	};

	csalt_store_read((csalt_static_store *)&data, 0, 0);
	if (!read_called) {
		print_error("Read call failed");
		return EXIT_FAILURE;
	}

	csalt_store_write((csalt_static_store *)&data, 0, 0);
	if (!write_called) {
		print_error("Write call failed");
		return EXIT_FAILURE;
	}

	csalt_store_split(
		(csalt_static_store *)&data,
		0,
		50,
		split_block,
		0
	);

	if (split_begin != 0 || split_end != 50) {
		print_error("Split failed, split_begin: %ld split_end: %ld", split_begin, split_end);
		return EXIT_FAILURE;
	}

	csalt_store_split(
		(csalt_static_store *)&data,
		10,
		20,
		split_block,
		0
	);

	if (split_begin != 10 || split_end != 20) {
		print_error("Split failed, split_begin: %ld split_end: %ld", split_begin, split_end);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

