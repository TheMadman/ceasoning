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

#ifndef CSALT_STORE_RWLOCK_H
#define CSALT_STORE_RWLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "decorator.h"

#include <csalt/platform/threads.h>

/**
 * \file
 * \copydoc csalt_store_rwlock
 */

/**
 * \brief A decorator synchronizing access to a store behind
 * 	a read/write lock.
 *
 * Write locks have higher precedence than read locks, and block
 * both read and write locks. Read locks only block write locks,
 * allowing multiple reads simultaneously.
 *
 * csalt_store_write() performs a write lock, csalt_store_read()
 * performs a read lock.
 *
 * csalt_store_split() splits the decorated store, decorates it
 * with the same lock, then passes that to the code block.
 *
 * csalt_store_size() performs a read lock and csalt_store_resize()
 * performs a write lock.
 */
struct csalt_store_rwlock {
	struct csalt_store_decorator parent;
	csalt_rwlock *lock;
};

/**
 * \public \memberof csalt_store_rwlock
 * \brief Constructor for a csalt_store_rwlock.
 */
struct csalt_store_rwlock csalt_store_rwlock(
	csalt_store *store,
	csalt_rwlock *lock
);

ssize_t csalt_store_rwlock_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
);

ssize_t csalt_store_rwlock_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
);

int csalt_store_rwlock_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
);

ssize_t csalt_store_rwlock_size(csalt_store *store);

ssize_t csalt_store_rwlock_resize(
	csalt_store *store,
	ssize_t new_size
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_STORE_RWLOCK_H
