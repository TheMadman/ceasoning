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

#ifndef CSALT_BASESTORES_H
#define CSALT_BASESTORES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <csalt/platform/init.h>

#include <csalt/util.h>

/**
 * \file
 * \brief This file defines interfaces for anything
 * which data can be written to or read from.
 *
 * These interfaces are intended to also represent
 * data stores which do not need to be acquired or
 * released, such as application global memory and stack
 * frame memory.
 */

/**
 * \brief Any struct whos first member is a pointer to
 * a csalt_static_store_interface can be passed with a basic
 * cast to the virtual functions taking csalt_static_store.
 */
typedef const struct csalt_static_store_interface * const csalt_static_store;

/**
 * \brief Any struct whos first member is a pointer to
 * a caslt_dynamic_store_interface can be passed with a basic
 * cast to the virtual functions taking csalt_store.
 */
typedef const struct csalt_dynamic_store_interface * const csalt_store;

/**
 * \brief Function type for reading data from a store into a buffer
 */
typedef ssize_t csalt_static_store_read_fn(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
);

/**
 * \brief Function type for writing data from a buffer into a store
 */
typedef ssize_t csalt_static_store_write_fn(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
);

/**
 * \brief Type for a logic block to use inside csalt_static_store_split_fn
 * functions.
 */
typedef int csalt_store_fn(csalt_static_store *store, void *data);

/**
 * \brief Function type for representing a sub-section of an
 * existing store as a new store, and performing an
 * action on the result.
 *
 * The last thing this function should do is return a
 * call to block(result, data) on success, or return an
 * error code on failure.
 *
 * \see csalt_static_store_split
 *
 */
typedef int csalt_static_store_split_fn(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_fn *block,
	void *data
);

/**
 * \brief Returns the size of the store, if applicable, or 0
 * if not applicable to this store.
 */
typedef ssize_t csalt_dynamic_store_size_fn(csalt_store *store);

/**
 * \brief Attempts to resize the store, then returns the size
 * of the store after the operation.
 *
 * Resizing the store can fail, in which case it may return
 * a different size than the one you have passed it.
 */
typedef ssize_t csalt_dynamic_store_resize_fn(
	csalt_store *store,
	ssize_t new_size
);

struct csalt_static_store_interface {
	csalt_static_store_read_fn *read;
	csalt_static_store_write_fn *write;
	csalt_static_store_split_fn *split;
};

struct csalt_dynamic_store_interface {
	struct csalt_static_store_interface store_interface;
	csalt_dynamic_store_size_fn *size;
	csalt_dynamic_store_resize_fn *resize;
};

/**
 * \public \memberof csalt_static_store
 * \brief Function for reading from a store into a buffer.
 *
 * Returns the amount of bytes actually read, or -1 on failure.
 *
 * \sa csalt_read()
 */
ssize_t csalt_static_store_read(csalt_static_store *store, void *buffer, ssize_t size);

/**
 * \public \memberof csalt_static_store
 * \brief Function for writing to a store from a buffer.
 *
 * Returns the amount of bytes actually written, or -1 on failure.
 *
 * \sa csalt_write()
 */
ssize_t csalt_static_store_write(csalt_static_store *store, const void *buffer, ssize_t size);

/**
 * \public \memberof csalt_static_store
 * \brief Provides the means to divide a store into a
 * sub-section and perform an operation on the result.
 *
 * One of the limitations of this function is to prevent
 * performing system calls (specifically, allocating heap
 * memory), unless used with stores which implement those
 * system calls. This limitation prevents some more conventional
 * approaches, including returning a pointer-to-store in
 * a nicely reentrant way; or using an out parameter pointer
 * if the caller does not know the structure of the returned
 * type (because it relies on polymorphic behaviour).
 *
 * For this reason, the use of this function is rather
 * unconventional - the first parameters are as expected:
 * the store you want to sub-section, and how you want to
 * sub-section it. The last two parameters are the logic
 * block to be performed on the resulting sub-section, and
 * additional data to pass to the logic block, for persisting
 * data after the return. This can be used in a
 * pseudo-recursive way to perform complex processing without
 * the overhead of additional system calls or the complexity
 * of resource (de-)allocation.
 *
 * The return value is the return value of the block parameter
 * and can be used for error handling.
 */
int csalt_static_store_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_fn *block,
	void *data
);

/**
 * \public \memberof csalt_store
 * \brief Returns the current size of the given store.
 */
ssize_t csalt_dynamic_store_size(csalt_store *store);

/**
 * \public \memberof csalt_store
 * \brief Attempts to resize the store to the given new_size, then returns
 * the size of the store once the operation is completed.
 *
 * Resizing a dynamic store can fail, in which case the returned value can
 * be different from the new_size passed.
 */
ssize_t csalt_dynamic_store_resize(
	csalt_store *store,
	ssize_t new_size
);

/**
 * \brief This structure allows the transfer algorithm to run in
 * a non-blocking fashion.
 *
 * When the remaining is equal to the total, the passed call-back is called,
 * allowing you to perform the same call in a loop (e.g. a render loop or a 
 * spin-lock in case you want blocking behaviour).
 *
 * \see csalt_progress()
 */
struct csalt_progress {
	ssize_t total;
	ssize_t amount_completed;
};

/**
 * \brief Creates a new struct csalt_progress with the total
 * set to amount and the amount_completed set to 0.
 */
struct csalt_progress csalt_progress(ssize_t amount);

/**
 * \brief Returns the remaining amount of data to transfer.
 */
ssize_t csalt_progress_remaining(const struct csalt_progress *progress);

/**
 * \brief This function returns truthy if progress is finished, false
 * otherwise
 */
int csalt_progress_complete(const struct csalt_progress *progress);

/**
 * \brief This function provides a convenient means to write data
 * from one store into another.
 *
 * If the transfer partially completes - for example, on
 * a non-blocking socket resource or similar - it returns
 * early, returning the total data transferred so far.
 *
 * Returns -1 on error.
 */
ssize_t csalt_static_store_transfer(
	struct csalt_progress *data,
	csalt_static_store *from,
	csalt_static_store *to
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_BASESTORES_H
