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

#include <csalt/util.h>
#include "test_macros.h"

#include <stdlib.h>

int receive_empty_split(csalt_store *store, void *data);
int receive_empty_first_pair(csalt_store *store, void *data);
int receive_split(csalt_store *store, void *data);
int receive_multisplit_standard(csalt_store *store, void *data);
int receive_multisplit_shorter_splits(csalt_store *store, void *data);

int main()
{
	{
		struct csalt_store_pair pair = csalt_store_pair(0, 0);
		ssize_t written = csalt_store_write(
			csalt_store(&pair),
			0,
			10
		);

		if (written > 0) {
			print_error(
				"csalt_store_write() returned a value on an empty pair: %ld",
				written
			);
			abort();
		}

		ssize_t read = csalt_store_read(
			csalt_store(&pair),
			0,
			10
		);

		if (read > 0) {
			print_error(
				"csalt_store_read() returned a value on an empty pair: %ld",
				read
			);
			abort();
		}

		csalt_store_split(
			csalt_store(&pair),
			0,
			10,
			receive_empty_split,
			0
		);
	}

	{
		struct csalt_store_stub stub = csalt_store_stub(1024);
		struct csalt_store_pair pair = csalt_store_pair(
			0,
			csalt_store(&stub)
		);

		ssize_t written = csalt_store_write(
			csalt_store(&pair),
			0,
			10
		);
		if (written != 10) {
			print_error("csalt_store_write() should have written to second store");
			abort();
		}

		ssize_t read = csalt_store_read(
			csalt_store(&pair),
			0,
			10
		);
		if (read != 10) {
			print_error("csalt_store_read() should have read from second store");
			abort();
		}

		csalt_store_split(
			csalt_store(&pair),
			0,
			10,
			receive_empty_first_pair,
			0
		);
	}

	{
		struct csalt_store_stub first = csalt_store_stub(1024);
		struct csalt_store_stub second = csalt_store_stub(512);

		csalt_store *stores[] = {
			csalt_store(&first),
			csalt_store(&second),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		if (pairs[0].second != (csalt_store *)&pairs[1]) {
			print_error(
				"First pair's second didn't point to second pair: %p vs. %p",
				(void *)pairs[0].second,
				(void *)&pairs[1]
			);
			abort();
		}

		if (pairs[1].second) {
			print_error("Last pair had non-null pointer value: %p", (void *)pairs[1].second);
			abort();
		}

		char data[8] = { 0 };

		ssize_t written_amount = csalt_store_write(csalt_store(pairs), data, sizeof(data));

		if (first.last_write != sizeof(data)) {
			print_error("Expected data to be written to first store");
			abort();
		}

		if (second.last_write != sizeof(data)) {
			print_error("Expected data to be written to second store");
			abort();
		}

		if (written_amount != sizeof(data)) {
			print_error("Returned written_amount was unexpected value: %ld", written_amount);
			abort();
		}

		csalt_store_read(csalt_store(pairs), data, sizeof(data));

		if (first.last_read != sizeof(data)) {
			print_error("Expected data to be read from first store");
			abort();
		}

		if (second.last_read != 0) {
			print_error("Expected no data to be read from second store");
			abort();
		}

		int result = csalt_store_split(
			csalt_store(pairs),
			5,
			10,
			receive_split,
			0
		);

		if (result != EXIT_SUCCESS) {
			print_error("Split failed");
			return result;
		}
	}

	{
		struct csalt_store_stub error = csalt_store_stub_error();
		struct csalt_store_stub success = csalt_store_stub(512);

		csalt_store *stores[] = {
			csalt_store(&error),
			csalt_store(&success),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		csalt_store *store = csalt_store(pairs);

		ssize_t read_amount = csalt_store_read(store, 0, 10);

		if (read_amount != -1) {
			print_error(
				"Got read when error was expected: %ld",
				read_amount
			);
			abort();
		}

		ssize_t write_amount = csalt_store_write(store, 0, 10);

		if (write_amount != -1) {
			print_error(
				"Got write when error was expected: %ld",
				write_amount
			);
			abort();
		}
	}

	{
		struct csalt_store_stub error = csalt_store_stub_error();
		struct csalt_store_stub zero = csalt_store_stub_zero();

		csalt_store *stores[] = {
			csalt_store(&zero),
			csalt_store(&error),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		csalt_store *store = csalt_store(pairs);

		ssize_t read_amount = csalt_store_read(store, 0, 10);

		if (read_amount != -1) {
			print_error(
				"Got read when error was expected: %ld",
				read_amount
			);
			abort();
		}

		ssize_t write_amount = csalt_store_write(store, 0, 10);

		if (write_amount != -1) {
			print_error(
				"Got write when error was expected: %ld",
				write_amount
			);
			abort();
		}
	}

	{
		struct csalt_store_stub first = csalt_store_stub(8);
		struct csalt_store_stub second = csalt_store_stub(8);

		csalt_store *stores[] = {
			csalt_store(&first),
			csalt_store(&second),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		ssize_t length = csalt_store_pair_list_length(pairs);

		if (length != 2l) {
			print_error(
				"Expected: length %ld actual: length %ld",
				2l,
				length
			);
			abort();
		}
	}

	{
		struct csalt_store_stub first = csalt_store_stub(256);
		struct csalt_store_stub second = csalt_store_stub(256);

		csalt_store *stores[] = {
			csalt_store(&first),
			csalt_store(&second),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		struct csalt_store_multisplit_split splits[] = {
			{ 0, 1 },
			{ 2, 3 },
		};

		int result = csalt_store_pair_list_multisplit(
			pairs,
			splits,
			receive_multisplit_standard,
			NULL
		);
	}

	{
		struct csalt_store_stub first = csalt_store_stub(256);
		struct csalt_store_stub second = csalt_store_stub(256);
		struct csalt_store_stub third = csalt_store_stub(256);
		struct csalt_store_stub fourth = csalt_store_stub(256);

		csalt_store *stores[] = {
			csalt_store(&first),
			csalt_store(&second),
			csalt_store(&third),
			csalt_store(&fourth),
		};

		struct csalt_store_pair pairs[arrlength(stores)] = { 0 };

		csalt_store_pair_list(stores, pairs);

		struct csalt_store_multisplit_split splits[] = {
			{ 0, 1 },
			{ 2, 3 },
		};

		int result = csalt_store_pair_list_multisplit(
			pairs,
			splits,
			receive_multisplit_shorter_splits,
			NULL
		);
	}
}

int receive_split(csalt_store *store, void *data)
{
	(void)data;
	struct csalt_store_pair *first_pair = (void *)store;
	struct csalt_store_pair *second_pair = (void *)first_pair->second;

	struct csalt_store_stub *first_stub = (void *)first_pair->first;
	struct csalt_store_stub *second_stub = (void *)second_pair->first;

	if (
		first_stub->split_begin != 5 ||
		first_stub->split_end != 10
	) {
		print_error(
			"First stub's bounds aren't right: %ld -> %ld",
			first_stub->split_begin,
			first_stub->split_end
		);
		abort();
	}

	if (
		second_stub->split_begin != 5 ||
		second_stub->split_end != 10
	) {
		print_error(
			"Second stub's bounds aren't right: %ld -> %ld",
			second_stub->split_begin,
			second_stub->split_end
		);
		abort();
	}

	return EXIT_SUCCESS;
}

int receive_empty_split(csalt_store *store, void *data)
{
	(void)store;
	(void)data;
	return 0;
}

int receive_empty_first_pair(csalt_store *store, void *data)
{
	struct csalt_store_pair *pair = (void *)store;

	if (pair->first || !pair->second) {
		print_error(
			"split pair with empty first member has unexpected value: %p -> %p",
			(void *)pair->first,
			(void *)pair->second
		);
		abort();
	}

	return 0;
}

int receive_multisplit_standard(csalt_store *store, void *data)
{
	struct csalt_store_pair *pair = (struct csalt_store_pair *)store;

	struct csalt_store_stub
		*zeroth = (void *)csalt_store_pair_list_get(pair, 0);
	struct csalt_store_stub
		*first = (void *)csalt_store_pair_list_get(pair, 1);

	if (zeroth->split_begin != 0 || zeroth->split_end != 1) {
		print_error(
			"Multisplit resulted in unexpected zeroth store: %ld -> %ld",
			zeroth->split_begin,
			zeroth->split_end
		);
		abort();
	}

	if (first->split_begin != 2 || first->split_end != 3) {
		print_error(
			"Multisplit resulted in unexpected first store: %ld -> %ld",
			first->split_begin,
			first->split_end
		);
		abort();
	}
}

int receive_multisplit_shorter_splits(csalt_store *store, void *data)
{
	struct csalt_store_pair *pair = (struct csalt_store_pair *)store;

	struct csalt_store_stub
		*zeroth = (void *)csalt_store_pair_list_get(pair, 0);
	struct csalt_store_stub
		*first = (void *)csalt_store_pair_list_get(pair, 1);
	struct csalt_store_stub
		*second = (void *)csalt_store_pair_list_get(pair, 2);
	struct csalt_store_stub
		*third = (void *)csalt_store_pair_list_get(pair, 3);

	if (zeroth->split_begin != 0 || zeroth->split_end != 1) {
		print_error(
			"Multisplit resulted in unexpected zeroth store: %ld -> %ld",
			zeroth->split_begin,
			zeroth->split_end
		);
		abort();
	}

	if (first->split_begin != 2 || first->split_end != 3) {
		print_error(
			"Multisplit resulted in unexpected first store: %ld -> %ld",
			first->split_begin,
			first->split_end
		);
		abort();
	}

	if (second->split_begin != 0 || second->split_end != 0) {
		print_error(
			"Multisplit resulted in unexpected second store: %ld -> %ld",
			second->split_begin,
			second->split_end
		);
		abort();
	}

	if (third->split_begin != 0 || third->split_end != 0) {
		print_error(
			"Multisplit resulted in unexpected third store: %ld -> %ld",
			third->split_begin,
			third->split_end
		);
		abort();
	}
}
