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

#include <csalt/fileresource.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "test_macros.h"

// Unlinkly to be duplicated by a human by accident
#define FILENAME "cay+5wGBIpGOM6DXxvAFDmqW"

int test_write_called = 0;
int test_read_called = 0;

void noop(csalt_store *dest)
{
	(void)dest;
}

int test_write(csalt_store *resource, void *filename)
{
	test_write_called = 1;
	int a = 1;
	struct csalt_memory A = csalt_memory_pointer(&a);

	struct csalt_progress transfer = csalt_progress(sizeof(a));
	ssize_t amount_written = csalt_store_transfer(
		&transfer,
		castto(csalt_store *, &A),
		castto(csalt_store *, resource),
		noop
	);
	if (amount_written != sizeof(a)) {
		print_error(
			"Unexpected number of bytes written, expected: %ld actual: %ld",
			sizeof(a),
			amount_written
		);

		// this whole function is skipped if the fd is invalid
		unlink(filename);
		exit(EXIT_FAILURE);
	}

	return 0;
}

int test_read(csalt_store *resource, void *filename)
{
	test_read_called = 1;
	int a = 0;
	struct csalt_memory A = csalt_memory_pointer(&a);

	struct csalt_progress transfer = csalt_progress(sizeof(a));
	ssize_t amount_read = csalt_store_transfer(
		&transfer,
		castto(csalt_store *, resource),
		castto(csalt_store *, &A),
		noop
	);
	if (amount_read != sizeof(a)) {
		print_error(
			"Unexpected number of bytes read, "
			"expected: %ld actual: %ld",
			sizeof(a),
			amount_read
		);
		unlink(filename);
		exit(EXIT_FAILURE);
	}
	if (!a) {
		print_error(
			"Read value didn't match expected value, "
			"expected: %d actual: %d",
			1,
			a
		);
		unlink(filename);
		exit(EXIT_FAILURE);
	}
	return 0;
}

int main()
{
	// first constructor -- creates file if not exists
	struct csalt_resource_file file = csalt_resource_create_file("./" FILENAME, O_RDWR, 0600);
	csalt_resource_use(csalt_resource(&file), test_write, "./" FILENAME);
	if (!test_write_called) {
		print_error("Write wasn't called when it should have been");
		unlink("./" FILENAME);
		return EXIT_FAILURE;
	}

	// second constructor -- file must exist before init
	struct csalt_resource_file file2 = csalt_resource_file("./" FILENAME, O_RDONLY);
	csalt_resource_use(csalt_resource(&file2), test_read, "./" FILENAME);
	if (!test_read_called) {
		print_error("Read wasn't called when it should have been");
		unlink("./" FILENAME);
		return EXIT_FAILURE;
	}

	unlink("./" FILENAME);

	return EXIT_SUCCESS;
}

