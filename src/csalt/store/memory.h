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

#ifndef CSALT_STORES_MEMORY_H
#define CSALT_STORES_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

/**
 * \file
 * \brief This module defines an interface for static memory,
 * 	such as globally-defined objects, statically-defined
 * 	objects, or stack-allocated objects.
 */

/**
 * \brief A structure for enabling access to a static block
 * 	of memory.
 */
struct csalt_store_memory {
	const struct csalt_static_store_interface *vtable;
	void *begin;
	void *end;
};

/**
 * \extends csalt_static_store
 * \brief Constructs a csalt_store_memory.
 *
 * \param begin The beginning of the memory block
 * \param end The end of the memory block
 *
 * \returns The store representing the memory block
 */
struct csalt_store_memory csalt_store_memory_bounds(void *begin, void *end);

#define csalt_store_memory(obj) \
	csalt_store_memory_bounds(&(obj), &(obj) + 1)
#define csalt_store_memory_array(array) \
	csalt_store_memory_bounds((array), csalt_arrend(array))

ssize_t csalt_store_memory_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount);

ssize_t csalt_store_memory_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount);

int csalt_store_memory_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param);

#ifdef __cplusplus
}  // extern "C"
#endif



#endif // CSALT_STORES_MEMORY_H
