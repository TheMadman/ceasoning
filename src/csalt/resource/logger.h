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

#ifndef CSALT_RESOURCE_LOGGER_H
#define CSALT_RESOURCE_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include <csalt/stores.h>
#include <csalt/util.h>

/**
 * \file
 * \copydoc csalt_resource_logger
 */

/**
 * \extends csalt_resource
 * \brief Decorates a resource with a logger, reporting
 * 	resource initialization failures.
 *
 * If the initialization succeeds, the resulting store is
 * also wrapped in a csalt_store_logger.
 */
struct csalt_resource_logger {
	const struct csalt_dynamic_resource_interface *vtable;
	csalt_resource *resource;

	struct csalt_store_logger result;
};

/**
 * \brief Constructs a csalt_resource_logger.
 *
 * If you want to log the success/failure of the resource
 * initialization, you should provide a csalt_log_message
 * for csalt_resource_init().
 *
 * To log messages for the store operations, simply
 * add them to the array as you would for csalt_store_logger.
 *
 * \param resource The resource to decorate
 * \param output The store to output logs to
 * \param error Error cases to log. If an empty array is provided,
 * 	no error cases are logged.
 * \param success Success cases to log. If an empty array is provided,
 * 	no success cases are logged.
 * \param partial_success Partially completed read/writes to log.
 * 	If an empty array is provided, no partial successes are logged.
 *
 * \returns The construct csalt_resource_logger.
 */
struct csalt_resource_logger csalt_resource_logger_arrays(
	csalt_resource *resource,
	csalt_static_store *output,
	struct csalt_array error,
	struct csalt_array success,
	struct csalt_array partial_success
);

csalt_store *csalt_resource_logger_init(csalt_resource *resource);
void csalt_resource_logger_deinit(csalt_resource *resource);

/**
 * \public \memberof csalt_resource_logger
 * \brief Takes an output resource and a C array of csalt_log_message%s
 * 	and constructs a new csalt_resource_logger.
 */
#define csalt_resource_logger_error(decorated, output, error_array) \
	csalt_resource_logger_arrays( \
		(decorated), \
		(output), \
		csalt_array(error_array), \
		(struct csalt_array) { 0 }, \
		(struct csalt_array) { 0 })

/**
 * \public \memberof csalt_resource_logger
 * \brief Takes an output resource and a C array of csalt_log_message%s
 * 	and constructs a new csalt_resource_logger.
 */
#define csalt_resource_logger_success(decorated, output, success_array) \
	csalt_resource_logger_arrays( \
		(decorated), \
		(output), \
		(struct csalt_array) { 0 }, \
		csalt_array(success_array), \
		(struct csalt_array) { 0 })

/**
 * \public \memberof csalt_resource_logger
 * \brief Takes an output resource and a C array of csalt_log_message%s
 * 	and constructs a new csalt_resource_logger.
 */
#define csalt_resource_logger_partial_success(decorated, output, ps_array) \
	csalt_resource_logger_arrays( \
		(decorated), \
		(output), \
		(struct csalt_array) { 0 }, \
		(struct csalt_array) { 0 }, \
		csalt_array(ps_array))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCE_LOGGER_H
