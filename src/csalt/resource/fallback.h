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

#ifndef CSALT_RESOURCES_FALLBACK_H
#define CSALT_RESOURCES_FALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "csalt/store/fallback.h"

/**
 * \file
 * \copydoc csalt_resource_fallback
 */

/**
 * \extends csalt_resource
 * \brief A resource that takes two resources and
 * 	produces a csalt_store_fallback with the
 * 	results.
 *
 * csalt_resource_init() attempts to initialize both
 * resources. If either fail, the whole resource returns
 * NULL, calling csalt_resource_deinit() on any resource
 * that did succeed.
 */
struct csalt_resource_fallback {
	const struct csalt_dynamic_resource_interface *vtable;
	csalt_resource *first;
	csalt_resource *second;
	struct csalt_store_fallback result;
};

/**
 * \public \memberof csalt_resource_fallback
 * \brief Constructs a csalt_resource_fallback from two
 * 	resources.
 *
 * Consider using csalt_resource_fallback_list() for a larger
 * list of resources.
 *
 * \param first The first resource
 * \param second The second resource
 *
 * \returns A new csalt_resource_fallback.
 */
struct csalt_resource_fallback csalt_resource_fallback(
	csalt_resource *first,
	csalt_resource *second
);

/**
 * \brief Creates a linked-list-like array of fallbacks.
 *
 * See csalt_resource_pair_list_bounds().
 * 
 * \param resource_begin The zeroth element of the resource array
 * \param resource_end A pointer one past the last element
 * 	of the resource array
 * \param fallback_begin The zeroth element of the fallback array
 * \param fallback_end A pointer one past the last element
 * 	of the fallback array
 *
 * \sa csalt_resource_fallback_list
 */
void csalt_resource_fallback_list_bounds(
	csalt_resource **resource_begin,
	csalt_resource **resource_end,
	struct csalt_resource_fallback *fallback_begin,
	struct csalt_resource_fallback *fallback_end
);

#define csalt_resource_fallback_list(resources, fallbacks) \
	csalt_resource_fallback_list_bounds( \
		(resources), \
		csalt_arrend(resources), \
		(fallbacks), \
		csalt_arrend(fallbacks))

csalt_store *csalt_resource_fallback_init(csalt_resource *resource);
void csalt_resource_fallback_deinit(csalt_resource *resource);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_FALLBACK_H
