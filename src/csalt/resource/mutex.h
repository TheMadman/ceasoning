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

#ifndef CSALT_RESOURCE_MUTEX_H
#define CSALT_RESOURCE_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

#include <csalt/store/mutex.h>
#include <csalt/platform/threads.h>

/**
 * \file
 * \copydoc csalt_resource_mutex
 */

/**
 * \extends csalt_resource
 * \brief Decorates a resource for synchronization with a mutex.
 *
 * csalt_resource_init() attempts to initialize both the resource
 * and the mutex. If either fails, NULL is returned. Otherwise,
 * a pointer to a csalt_store_mutex is returned, wrapping the
 * initialized resource.
 */
struct csalt_resource_mutex {
	const struct csalt_dynamic_resource_interface *vtable;
	csalt_resource *resource;
	csalt_mutex_params *params;
	csalt_mutex mutex;
	struct csalt_store_mutex result;
};

/**
 * \public \memberof csalt_resource_mutex
 * \brief Constructs a csalt_resource_mutex.
 *
 * \param resource The resource to decorate.
 * \param params Initialization parameter for the mutex.
 * 	This may vary from operating system to operating system.
 *
 * \returns The newly constructed csalt_resource_mutex.
 */
struct csalt_resource_mutex csalt_resource_mutex(
	csalt_resource *resource,
	csalt_mutex_params *params
);

csalt_store *csalt_resource_mutex_init(csalt_resource *resource);
void csalt_resource_mutex_deinit(csalt_resource *resource);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCE_MUTEX_H
