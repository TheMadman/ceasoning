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

#include "test_macros.h"

#include "csalt/resources.h"

#include <string.h>

#define MESSAGE "Hello, world!"

int block(csalt_store *, void *);

int main()
{
	int result = csalt_use_format(block, NULL, "%s", MESSAGE);
	if (result != 0)
		print_error_and_exit("Unexpected result: %d", result);

	return EXIT_SUCCESS;
}

int block(csalt_store *result, void *_)
{
	(void)_;
	char buffer[sizeof(MESSAGE)] = { 0 };
	ssize_t read_amount = csalt_store_read((csalt_static_store*)result, buffer, sizeof(buffer));
	if (read_amount != sizeof(buffer))
		print_error_and_exit("Unexpected read_amount: %ld", read_amount);

	if (strcmp(buffer, MESSAGE) != 0)
		print_error_and_exit("Unexpected buffer value: %s", buffer);

	return 0;
}
