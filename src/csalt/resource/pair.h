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

#ifndef CSALT_RESOURCES_PAIR_H
#define CSALT_RESOURCES_PAIR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "csalt/store/pair.h"

/**
 * \file
 * \copydoc csalt_resource_pair
 */

/**
 * \brief Treats two pairs as a single pair.
 *
 * csalt_resource_init() attempts to initialize
 * the first resource. If that returns NULL, this returns NULL.
 * Otherwise, it attempts to initialize the second
 * resource. If that returns NULL, the first resource
 * is freed, then this returns NULL. If both succeed, the
 * result is wrapped in a csalt_store_pair and returned.
 *
 * csalt_resource_deinit() deinitializes the second resource,
 * then the first resource.
 */
struct csalt_resource_pair {
	const struct csalt_dynamic_resource_interface *vtable;
	csalt_resource *first;
	csalt_resource *second;
	struct csalt_store_pair result;
};

/**
 * \public \memberof csalt_resource_pair
 * \brief Constructor for a csalt_resource_pair.
 *
 * \param first The first resource
 * \param second The second resource
 *
 * \returns The new csalt_resource_pair.
 */
struct csalt_resource_pair csalt_resource_pair(
	csalt_resource *first,
	csalt_resource *second
);

/**
 * \brief Creates a linked-list-like array of pairs.
 *
 * This function effectively constructs a linked-list from pairs.
 * The first pair's first property points to the first store in
 * the array. The second property points to the second pair.
 * The second pair's first property points to the second store,
 * and its second property points to the next pair, and so on.
 * The last pair's second property is set to a null pointer value.
 *
 * Note that this function uses an out-parameter for initializing an
 * already-existing array, instead of returning a value.
 *
 * Constructing an array for an existing local array of store is simple:
 * \code
 * 	struct csalt_resource_pair list[arrsize(resource)] = { 0 };
 * \endcode
 *
 * The output array is untouched if there was an error with the
 * parameters, such as the input/output being zero length, or the
 * output array being smaller than the input array 
 *
 * \param resource_begin The zeroth element of the resource array
 * \param resource_end A pointer one past the end of the resource array
 * \param pairs_begin The zeroth element of the pairs array
 * \param pairs_end A pointer one past the end of the pairs array
 *
 * \sa csalt_resource_pair_list()
 */
void csalt_resource_pair_list_bounds(
	csalt_resource **resource_begin,
	csalt_resource **resource_end,
	struct csalt_resource_pair *pairs_begin,
	struct csalt_resource_pair *pairs_end
);

/**
 * \brief Convenience macro for constructing a pair_list from two arrays.
 *
 * This macro is the recommended way of constructing a pair_list, given
 * the two arrays' sizes are defined at compile-time. Dynamic arrays, such as
 * those allocated with malloc() or calloc(), must still use
 * csalt_store_pair_list_bounds().
 *
 * \sa csalt_resource_pair_list_bounds()
 */
#define csalt_resource_pair_list(resource_array, pair_array) \
	csalt_resource_pair_list_bounds( \
		(resource_array), \
		arrend(resource_array), \
		(pair_array), \
		arrend(pair_array) \
	)

csalt_store *csalt_resource_pair_init(csalt_resource *resource);
void csalt_resource_pair_deinit(csalt_resource *resource);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_PAIR_H
