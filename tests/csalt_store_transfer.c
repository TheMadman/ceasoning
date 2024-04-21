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

#include "test_macros.h"
#include "csalt/util.h"

#include <string.h>
#include <stdlib.h>

#define ARRSIZE 1 << 20
char c[ARRSIZE], d[ARRSIZE];
int transfer_complete_big_called = 0;

int main()
{
	int a = 0, b = 1;

	struct csalt_store_memory
		A = csalt_store_memory(a),
		B = csalt_store_memory(b);

	struct csalt_progress transfer = csalt_progress(sizeof(a));
	csalt_store_transfer(
		&transfer,
		(csalt_static_store *)&A,
		(csalt_static_store *)&B
	);

	if (a != b)
		print_error_and_exit("A -> B transfer failed: %d != %d", a, b);

	// test larger-than-page values
	memset(c, 1, ARRSIZE);
	memset(d, 0, ARRSIZE);

	struct csalt_store_memory
		C = csalt_store_memory_array(c),
		D = csalt_store_memory_array(d);

	transfer = csalt_progress(ARRSIZE);
	ssize_t transfer_amount = 0;
	while ((transfer_amount = csalt_store_transfer(
		&transfer,
		(csalt_static_store *)&C,
		(csalt_static_store *)&D
	)) < ARRSIZE) {
		switch (transfer_amount) {
			case -1:
				print_error("Error code returned");
				return EXIT_FAILURE;
			case 0:
				print_error("Zero bytes transfered");
				return EXIT_FAILURE;
			case ARRSIZE:
				break;
		}
	}

	for (char *test = d; test < &d[ARRSIZE]; test++) {
		if (*test != 1) {
			print_error("Byte %ld doesn't match expected value: %c", test - d, *test);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
