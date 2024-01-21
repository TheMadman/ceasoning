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

#ifndef CSALT_STORE_MUTEX_H
#define CSALT_STORE_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "decorator.h"

#include <csalt/platform/threads.h>

/**
 * \file
 * \copydoc csalt_store_mutex
 */

/**
 * \extends csalt_store_decorator
 * \brief Provides a decorator for synchronizing access to a store.
 *
 * Locks are attempted in a non-blocking fashion; if the lock fails,
 * the functions immediately return -1.
 *
 * - csalt_store_read() and csalt_store_write() are synchronized
 * - csalt_store_split() causes the mutex to be locked, and the
 *   decorated store to be passed, undecorated, to the block. This
 *   acts as a transaction interface for the lock, preventing deadlock
 *   and allowing multiple other operations to be performed under a
 *   single lock.
 *
 * Bear in mind that calling csalt_store_size() separately and trying
 * to use the result in a multi-threaded context will always result
 * in a race, and csalt_store_split() produces a csalt_static_store,
 * which cannot have csalt_store_size() or csalt_store_resize() on it.
 *
 * The intended use of csalt_store#s is to use the size/resize calls
 * in a strictly serial manner and, once finished, to then perform
 * splits and pass the split results to separate threads.
 */
struct csalt_store_mutex {
	struct csalt_store_decorator parent;
	csalt_mutex *mutex;
};

/**
 * \public \memberof csalt_store_mutex
 * \brief Constructs a csalt_store_mutex.
 *
 * \param decorated The store to decorate
 * \param mutex The mutex to use as a lock
 *
 * \returns A constructed csalt_store_mutex.
 */
struct csalt_store_mutex csalt_store_mutex(
	csalt_store *decorated,
	csalt_mutex *mutex);

ssize_t csalt_store_mutex_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount);
ssize_t csalt_store_mutex_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount);
int csalt_store_mutex_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_STORE_MUTEX_H
