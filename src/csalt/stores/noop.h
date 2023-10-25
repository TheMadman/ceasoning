/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2023   Marcus Harrison
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

/**
 * \file
 * \brief This file is responsible for defining the no-op store and
 * static-store interface.
 */

#ifndef CSALT_STORES_NOOP_H
#define CSALT_STORES_NOOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "csalt/stores/base.h"

#include <stdbool.h>

/**
 * \brief A store representing a no-op interface, useful for
 * 	error cases.
 *
 * csalt_static_store_read() and csalt_static_store_write()
 * return -1. csalt_static_store_split() simply calls the passed
 * block with the no-op store. csalt_store_size() and
 * csalt_store_resize() return -1.
 *
 * There will only ever be one no-op store in the program,
 * statically allocated. Both csalt_store_noop and
 * csalt_store_static_noop use the same store, so only one
 * check is required.
 */
extern csalt_store * const csalt_store_noop;

/**
 * \copydoc csalt_store_noop
 */
extern csalt_static_store * const csalt_static_store_noop;

/**
 * \brief Returns true if the given store failed to initialize,
 * or initialized to a no-op store, or false otherwise.
 *
 * A failure to initialize could mean that the pointer itself
 * is null, e.g. because a csalt_resource returned null from
 * csalt_resource_init(), or it could mean that the store
 * was initialized to the no-op interface.
 */
bool csalt_store_error(const csalt_store * const store);

/**
 * \copydoc csalt_store_error()
 */
bool csalt_static_store_error(const csalt_static_store * const store);

/**
 * \brief The implementation of csalt_static_store_read() for no-op stores.
 */
ssize_t csalt_store_noop_read(csalt_static_store *, void *, ssize_t);

/**
 * \brief The implementation of csalt_static_store_write() for no-op stores.
 */
ssize_t csalt_store_noop_write(csalt_static_store *, void *, ssize_t);

/**
 * \brief The implementation of csalt_static_store_split() for no-op stores.
 */
ssize_t csalt_store_noop_split(
	csalt_static_store *,
	ssize_t,
	ssize_t,
	csalt_store_fn*,
	void *
);

/**
 * \brief The implementation of csalt_store_size() for no-op stores.
 */
ssize_t csalt_store_noop_size(csalt_store *);

/**
 * \brief The implementation of csalt_store_resize() for no-op stores.
 */
ssize_t csalt_store_noop_resize(csalt_store *, ssize_t);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_STORES_NOOP_H
