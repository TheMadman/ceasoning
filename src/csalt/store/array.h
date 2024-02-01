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

#ifndef CSALT_STORES_ARRAY_H
#define CSALT_STORES_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "decorator.h"

#include <stdbool.h>

/**
 * \file
 * \brief This module provides a decorator which converts
 * 	read/write/split/size/resize sizes into C-array-like commands, and
 * 	also provides special read/write functions for reading from/writing
 * 	to a specified index.
 */

/**
 * \extends csalt_store_decorator
 * \brief This decorator translates requests to the wrapped store using array-like
 * 	semantics.
 *
 * Specifically, it translates requests from byte-oriented to object-oriented,
 * where objects are of a fixed size:
 *
 * - For csalt_store_read() and csalt_store_write(), the `size` argument is
 *   	multiplied by the object size;
 * - For csalt_store_split(), the `begin` and `end` arguments are multiplied
 *   	by the object size, and calls `block` with a store which is **not**
 *   	array-decorated;
 * - For csalt_store_size(), the return value is the number of objects;
 * - For csalt_store_resize(), the `size` argument is multiplied by the object
 *   	size, and the return value is the number of objects.
 *
 * The reason csalt_store_split returns the decorated store _without_ an
 * array decorator is to more easily allow re-use of functions: array decoration
 * must only be performed once, otherwise it will be compounded on each new
 * decoration.
 */
struct csalt_store_array {
	struct csalt_store_decorator parent;
	ssize_t object_size;
};

/**
 * \public \memberof csalt_store_array
 * \brief Constructs a new array decorator.
 *
 * \param store The store to be decorated
 * \param object_size The size of each object
 *
 * \returns A new array decorator.
 */
struct csalt_store_array csalt_store_array(
	csalt_store *store,
	ssize_t object_size);

ssize_t csalt_store_array_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size);
ssize_t csalt_store_array_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size);
int csalt_store_array_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param);
ssize_t csalt_store_array_size(csalt_store *store);
ssize_t csalt_store_array_resize(csalt_store *store, ssize_t new_size);

/**
 * \public \memberof csalt_store_array
 * \brief Writes a single object to the given index of the array.
 *
 * \param array The store to modify
 * \param index The index to write to
 * \param buffer A pointer to the object to write
 *
 * \returns true on success, false on failure.
 */
bool csalt_store_array_set(
	struct csalt_store_array *array,
	ssize_t index,
	const void *buffer);

/**
 * \public \memberof csalt_store_array
 * \brief Reads a single object from the given index of the array.
 *
 * Note that no checks are done to see if the index is correct or initialized.
 *
 * \param array The store to read
 * \param index The index to read from
 * \param buffer A pointer to the buffer to write to
 *
 * \returns true on success, false on failure.
 */
bool csalt_store_array_get(
	const struct csalt_store_array *array,
	ssize_t index,
	void *buffer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_STORES_ARRAY_H
