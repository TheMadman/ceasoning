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

#include "csalt/stores.h"

#include <string.h>
#include <stdbool.h>

typedef struct csalt_dynamic_store_stub stub_t;
typedef struct csalt_store_logger logger_t;

bool prefix_match(const char *expected, const char *actual)
{
	return strstr(actual, expected) == actual;
}

int main()
{
	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		stub_t stub = csalt_dynamic_store_stub_error();

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_store_read, "test read"),
			csalt_log_message(csalt_store_write, "test write"),
			csalt_log_message(csalt_store_resize, "test resize"),
		};

		logger_t logger = csalt_store_logger_error(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_static_store *store = (void*)&logger;

		csalt_store_read(store, NULL, 10);

		const char expected_read[] = "test read: csalt_store_read(";
		if (!prefix_match(expected_read, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);

		csalt_store_write(store, NULL, 10);

		const char expected_write[] = "test write: csalt_store_write(";
		if (!prefix_match(expected_write, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);

		csalt_store_resize((csalt_store*)store, 10);

		const char expected_resize[] = "test resize: csalt_store_resize(";
		if (!prefix_match(expected_resize, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}

	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		stub_t stub = csalt_dynamic_store_stub(4096);

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_store_read, "test read"),
			csalt_log_message(csalt_store_write, "test write"),
			csalt_log_message(csalt_store_resize, "test resize"),
		};

		logger_t logger = csalt_store_logger_success(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_static_store *store = (void*)&logger;

		csalt_store_read(store, NULL, 10);

		const char expected_read[] = "test read: csalt_store_read(";
		if (!prefix_match(expected_read, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);

		csalt_store_write(store, NULL, 10);

		const char expected_write[] = "test write: csalt_store_write(";
		if (!prefix_match(expected_write, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);

		csalt_store_resize((csalt_store*)store, 10);

		const char expected_resize[] = "test resize: csalt_store_resize(";
		if (!prefix_match(expected_resize, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}
}
