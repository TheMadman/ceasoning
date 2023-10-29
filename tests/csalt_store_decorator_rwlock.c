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

csalt_rwlock test_lock;

ssize_t stub_rwlock_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
)
{
	int lock_failed = csalt_rwlock_tryrdlock(&test_lock);
	if (lock_failed)
		return -1;

	int write_lock_failed = csalt_rwlock_trywrlock(&test_lock);
	if (!write_lock_failed)
		return -1;
	return amount;
}

ssize_t stub_rwlock_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
)
{
	int lock_failed = csalt_rwlock_trywrlock(&test_lock);
	if (!lock_failed)
		return -1;
	return amount;
}

int stub_rwlock_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	int lock_failed = csalt_rwlock_trywrlock(&test_lock);
	if (!lock_failed)
		return -1;
	return 0;
}

struct csalt_store_interface stub_rwlock_implementation = {
	stub_rwlock_read,
	stub_rwlock_write,
	0,
	stub_rwlock_split,
};

int main()
{
	{
		int lock_init_failure = csalt_rwlock_init(&test_lock, 0);
		if (lock_init_failure)
			return EXIT_TEST_ERROR;

		csalt_store stub = &stub_rwlock_implementation;

		struct csalt_store_decorator_rwlock
			decorator = csalt_store_decorator_rwlock(
				&stub,
				&test_lock
			);

		ssize_t result = csalt_store_read(
			csalt_store(&decorator),
			0,
			0
		);
		if (result < 0) {
			print_error("Read lock wasn't acquired correctly");
			return EXIT_FAILURE;
		}

		csalt_rwlock_deinit(&test_lock);
	}

	{
		int lock_init_failure = csalt_rwlock_init(&test_lock, 0);
		if (lock_init_failure)
			return EXIT_TEST_ERROR;

		csalt_store stub = &stub_rwlock_implementation;

		struct csalt_store_decorator_rwlock
			decorator = csalt_store_decorator_rwlock(
				&stub,
				&test_lock
			);

		ssize_t result = csalt_store_write(
			csalt_store(&decorator),
			0,
			0
		);
		if (result < 0) {
			print_error("Write lock wasn't acquired correctly");
			return EXIT_FAILURE;
		}

		csalt_rwlock_deinit(&test_lock);
	}

	{
		int lock_init_failure = csalt_rwlock_init(&test_lock, 0);
		if (lock_init_failure)
			return EXIT_TEST_ERROR;

		csalt_store stub = &stub_rwlock_implementation;

		struct csalt_store_decorator_rwlock
			decorator = csalt_store_decorator_rwlock(
				&stub,
				&test_lock
			);

		ssize_t result = csalt_store_split(
			csalt_store(&decorator),
			0,
			0,
			0,
			0
		);
		if (result < 0) {
			print_error("Write lock wasn't acquired correctly");
			return EXIT_FAILURE;
		}

		csalt_rwlock_deinit(&test_lock);
	}


	return EXIT_SUCCESS;
}
