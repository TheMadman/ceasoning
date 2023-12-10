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

#ifndef CSALT_HEAP_H
#define CSALT_HEAP_H

/**
 * \file This file describes a heap resource which can be resized.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

/*
 * Used for the type returned by csalt_resource_heap.
 * This should not be considered part of the public API.
 */
struct csalt_store_heap {
	const struct csalt_dynamic_store_interface *vtable;
	char *begin;
	char *end;
	ssize_t written;
};

/**
 * \extends csalt_resource
 * \brief Represents a request to allocate heap memory.
 */
struct csalt_resource_heap {
	const struct csalt_resource_interface *vtable;
	ssize_t size;
	struct csalt_store_heap store;
};

/**
 * \public \memberof csalt_resource_heap
 * \brief Constructs a new csalt_resource_heap.
 *
 * \param initial_size The initial size of the heap. The returned store
 * 	can be resized.
 *
 * \returns A constructed heap resource
 */
struct csalt_resource_heap csalt_resource_heap(ssize_t initial_size);

csalt_store *csalt_resource_heap_init(csalt_resource *);
void csalt_resource_heap_deinit(csalt_resource *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_HEAP_H
