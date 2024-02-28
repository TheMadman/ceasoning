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

#include "test_macros.h"

#include <csalt/resources.h>

INIT_IMPL(
	int,
	getaddrinfo,
	ARGS(
		const char *node,
		const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res
	),
	ARGS(
		node,
		service,
		hints,
		res
	)
);

INIT_IMPL(
	void,
	freeaddrinfo,
	struct addrinfo *res,
	res
);

int getaddrinfo_fail(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
)
{
	return -1;
}

struct addrinfo success_result = {
	.ai_next = NULL
};

int getaddrinfo_success(
	const char *node,
	const char *service,
	const struct addrinfo *hints,
	struct addrinfo **res
)
{
	*res = &success_result;
	return 0;
}

void freeaddrinfo_noop(struct addrinfo *res)
{
	(void)res;
}

int expect_failure(const struct addrinfo *result, void *param)
{
	(void)result;
	(void)param;
	print_error_and_exit("expect_failure should never be called");
	return -1;
}

int expect_success(const struct addrinfo *result, void *param)
{
	(void)param;
	if (result != &success_result)
		print_error_and_exit("result was unexpected value, expected: %p got: %p", &success_result, result);
	return 0;
}

int main()
{
	SET_IMPL(freeaddrinfo, freeaddrinfo_noop);
	{
		SET_IMPL(getaddrinfo, getaddrinfo_fail);
		const int result = csalt_resource_network_getaddrinfo(
			"example.com",
			"443",
			NULL,
			expect_failure,
			NULL);

		if (result != -1)
			print_error_and_exit("result was unexpected value, expected: %d got: %d", -1, result);
	}

	{
		SET_IMPL(getaddrinfo, getaddrinfo_success);
		const int result = csalt_resource_network_getaddrinfo(
			"example.com",
			"443",
			NULL,
			expect_success,
			NULL);

		if (result != 0)
			print_error_and_exit("result was unexpected value, expected: %d got: %d", 0, result);
	}
}
