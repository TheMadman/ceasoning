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

#include <csalt/resources.h>
#include <csalt/stores.h>

#include <string.h>
#include <stdbool.h>

#include "test_macros.h"

typedef struct csalt_resource_logger logger_t;

bool prefix_match(const char *expected, const char *actual)
{
	return strstr(actual, expected) == actual;
}

int main()
{
	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		struct csalt_resource_stub
			stub = csalt_resource_stub(0);

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_resource_init, "success resource"),
		};

		logger_t logger = csalt_resource_logger_success(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_resource_init((void*)&logger);

		static const char expected[] = "success resource: csalt_resource_init(";
		if (!prefix_match(expected, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}

	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		struct csalt_resource_stub
			stub = csalt_resource_stub(1);

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_resource_init, "failure resource"),
		};

		logger_t logger = csalt_resource_logger_error(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_resource_init((void*)&logger);

		static const char expected[] = "failure resource: csalt_resource_init(";
		if (!prefix_match(expected, buffer))
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}

	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		struct csalt_resource_stub
			stub = csalt_resource_stub(0);

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_resource_init, "success resource"),
		};

		logger_t logger = csalt_resource_logger_error(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_resource_init((void*)&logger);
		if (buffer[0] != '\0')
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}

	{
		char buffer[4096] = { 0 };
		struct csalt_store_memory memory = csalt_store_memory_array(buffer);
		struct csalt_resource_stub
			stub = csalt_resource_stub(1);

		struct csalt_log_message messages[] = {
			csalt_log_message(csalt_resource_init, "failure resource"),
		};

		logger_t logger = csalt_resource_logger_success(
			(void*)&stub,
			(void*)&memory,
			messages);

		csalt_resource_init((void*)&logger);
		if (buffer[0] != '\0')
			print_error_and_exit("Log was unexpected value: \"%s\"", buffer);
	}
}

