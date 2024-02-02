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

#ifndef CSALT_RESOURCES_FIRST_H
#define CSALT_RESOURCES_FIRST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pair.h"

/**
 * \file
 * \copydoc csalt_resource_first
 */

/**
 * \brief A resource which returns the store of
 * 	the first resource to initialize successfully.
 *
 * On csalt_resource_init(), the first resource is tried.
 * If the first resource initializes successfully, its
 * store is returned without initializing the second
 * resource.
 *
 * If the first resource fails to initialize, the second
 * resource is attempted. On success, its store is returned.
 * Otherwise, NULL is returned.
 */
struct csalt_resource_first {
	struct csalt_resource_pair parent;
	csalt_resource *initialized;
};

/**
 * \public \memberof csalt_resource_first
 * \brief Constructor for a csalt_resource_first.
 *
 * \param first The first resource to try
 * \param second The second resource to try
 *
 * \returns The resulting first resource.
 * \sa csalt_resource_first_list()
 */
struct csalt_resource_first csalt_resource_first(
	csalt_resource *first,
	csalt_resource *second
);

/**
 * \brief Initializes a linked list of csalt_resource_first#s.
 *
 * See csalt_resource_pair_list_bounds().
 *
 * \param resource_begin The zeroth element of the resource array
 * \param resource_end A pointer one past the last element
 * 	of the resource array
 * \param first_begin The zeroth element of the first array
 * \param first_end A pointer one past the last element
 * 	of the first array
 *
 * \sa csalt_resource_first_list()
 */
void csalt_resource_first_list_bounds(
	csalt_resource **resource_begin,
	csalt_resource **resource_end,
	struct csalt_resource_first *first_begin,
	struct csalt_resource_first *first_end
);

/**
 * \brief Convenience macro for constructing a first_list from two arrays.
 *
 * This macro is the recommended way of constructing a first_list, given
 * the two arrays' sizes are defined at compile-time. Dynamic arrays, such as
 * those allocated with malloc() or calloc(), must still use
 * csalt_store_first_list_bounds().
 *
 * \sa csalt_resource_first_list_bounds()
 */
#define csalt_resource_first_list(resource_array, first_array) \
	csalt_resource_first_list_bounds( \
		(resource_array), \
		arrend(resource_array), \
		(first_array), \
		arrend(first_array) \
	)

csalt_store *csalt_resource_first_init(csalt_resource *);
void csalt_resource_first_deinit(csalt_resource *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_FIRST_H
