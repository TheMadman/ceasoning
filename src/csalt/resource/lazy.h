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

#ifndef CSALT_RESOURCE_LAZY_H
#define CSALT_RESOURCE_LAZY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "csalt/stores.h"

/**
 * \file
 * \copydoc csalt_resource_lazy
 */

/**
 * \extends csalt_resource
 * \brief This resource takes a resource and delays initialization
 * 	until the store is used.
 *
 * csalt_resource_init() does not initialize the wrapped resource,
 * instead returning a store.
 *
 * Each time the store is used, if the resource has not been initialized,
 * csalt_resource_init() is called on the wrapped resource. If the
 * initialization fails, the store functions return an error code.
 * Otherwise, the returned store is used.
 *
 * csalt_resource_deinit() checks if the wrapped resource was initialized
 * and deinitializes it if it was.
 */
struct csalt_resource_lazy {
	const struct csalt_dynamic_resource_interface *vtable;
	struct csalt_store_lazy {
		const struct csalt_dynamic_store_interface *vtable;
		csalt_resource *resource;
		csalt_store *store;
	} result;
};

/**
 * \public \memberof csalt_resource_lazy
 * \brief Constructs a csalt_resource_lazy.
 *
 * \param resource The resource to decorate
 *
 * \returns The constructed csalt_resource_lazy
 */
struct csalt_resource_lazy csalt_resource_lazy(csalt_resource *resource);

csalt_store *csalt_resource_lazy_init(csalt_resource *resource);
void csalt_resource_lazy_deinit(csalt_resource *resource);

ssize_t csalt_store_lazy_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
);
ssize_t csalt_store_lazy_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
);
int csalt_store_lazy_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
);
ssize_t csalt_store_lazy_size(csalt_store *store);
ssize_t csalt_store_lazy_resize(csalt_store *store, ssize_t new_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCE_LAZY_H
