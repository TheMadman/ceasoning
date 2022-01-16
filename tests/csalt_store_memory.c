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

int main()
{
	struct csalt_store_stub stub = csalt_store_stub(1024);
	int data = 42;

	ssize_t write = csalt_write(&stub, data);

	if (write < 0) {
		print_error("Unexpected error writing");
		return EXIT_FAILURE;
	}

	if (write != sizeof(data)) {
		print_error("Unexpected csalt_write return value: %ld", write);
		return EXIT_FAILURE;
	}

	data = 0;

	ssize_t read = csalt_read(&stub, data);

	if (read < 0) {
		print_error("Unexpected error reading");
		return EXIT_FAILURE;
	}

	if (read != sizeof(data)) {
		print_error("Unexpected csalt_read return value: %ld", read);
		return EXIT_FAILURE;
	}
}
