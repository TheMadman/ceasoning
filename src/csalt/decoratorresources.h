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

#ifndef DECORATORRESOURCES_H
#define DECORATORRESOURCES_H

#include <csalt/platform/init.h>

#include "baseresource.h"
#include "decoratorresource.h"
#include "util.h"
#include "csalt/platform/threads.h"

/**
 * \file
 * \brief This file is responsible for providing decorator functions around
 * csalt_resource%s.
 */

/**
 * \brief Default forwarding function for initializing a resource.
 */
csalt_store *csalt_resource_decorator_init(csalt_resource *);

/**
 * \brief Default fowarding function for deinitializing a resource.
 */
void csalt_resource_decorator_deinit(csalt_resource *);

struct csalt_resource_decorator {
	const struct csalt_dynamic_resource_interface *vtable;
	csalt_resource *child;

	struct csalt_store_decorator initialized;
};

/**
 * \brief Decorator which wraps a resource, logs messages for
 * 	resource operations and, on successful initialization,
 * 	wraps the resulting store in a csalt_store_decorator_logger.
 */
struct csalt_resource_decorator_logger {
	struct csalt_resource_decorator decorator;

	struct csalt_store_decorator_logger store_logger;
};

/**
 * \brief Constructor for csalt_resource_decorator_logger, taking
 * 	similar arguments to csalt_store_decorator_logger(). This
 * 	decorator also supports setting a message for the csalt_resource_init()
 * 	function.
 *
 * The resource only checks for error or success states on csalt_resource_init.
 * If the resource is initialized successfully, a pointer to a
 * csalt_store_decorator_logger is passed, which has all the message lists passed
 * to it, enabling logging on the resulting store as well.
 *
 * \see csalt_resource_decorator_logger()
 * \see csalt_resource_decorator_logger_error_bounds()
 */
struct csalt_resource_decorator_logger csalt_resource_decorator_logger_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
);

/**
 * \brief Convenience macro for setting up a logger using arrays.
 */
#define csalt_resource_decorator_logger( \
	child, \
	file_descriptor, \
	errors_array, \
	successes_array, \
	zery_bytes_array \
) \
csalt_resource_decorator_logger_bounds( \
	(child), \
	(file_descriptor), \
	(errors_array), \
	csalt_arrend(errors_array), \
	(successes_array), \
	csalt_arrend(successes_array), \
	(zero_bytes_array), \
	csalt_arrend(zero_bytes_array) \
)

/**
 * \brief A convenience constructor for setting up an error-only logger.
 *
 * \see csalt_resource_decorator_logger_errors()
 * \see csalt_resource_decorator_logger_bounds()
 */
struct csalt_resource_decorator_logger csalt_resource_decorator_logger_error_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end
);

/**
 * \brief A convenience macro fo setting up an error-only logger using arrays.
 *
 * \see csalt_resource_decorator_logger_error_bounds()
 * \see csalt_resource_decorator_logger_bounds()
 */
#define csalt_resource_decorator_logger_errors(child, fd, errors_array) \
	csalt_resource_decorator_logger_error_bounds( \
		(child), \
		(fd), \
		(errors_array), \
		csalt_arrend(errors_array) \
	)

/**
 * \brief A convenience constructor for setting up a success-only logger.
 *
 * \see csalt_resource_decorator_logger_bounds()
 */
struct csalt_resource_decorator_logger csalt_resource_decorator_logger_success_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *successs_begin,
	const struct csalt_store_log_message *successs_end
);

/**
 * \brief A convenience macro fo setting up an success-only logger using arrays.
 *
 * \see csalt_resource_decorator_logger_success_bounds()
 * \see csalt_resource_decorator_logger_bounds()
 */
#define csalt_resource_decorator_logger_successes(child, fd, successs_array) \
	csalt_resource_decorator_logger_success_bounds( \
		(child), \
		(fd), \
		(successs_array), \
		csalt_arrend(successs_array) \
	)

/**
 * \brief A convenience constructor for setting up a zero-bytes-only logger.
 *
 * \see csalt_resource_decorator_logger_bounds()
 */
struct csalt_resource_decorator_logger csalt_resource_decorator_logger_zero_bytes_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *zero_bytess_begin,
	const struct csalt_store_log_message *zero_bytess_end
);

/**
 * \brief A convenience macro fo setting up an zero-bytes-only logger using arrays.
 *
 * \see csalt_resource_decorator_logger_zero_bytes_bounds()
 * \see csalt_resource_decorator_logger_bounds()
 */
#define csalt_resource_decorator_logger_zero_bytes(child, fd, successs_array) \
	csalt_resource_decorator_logger_zero_bytes_bounds( \
		(child), \
		(fd), \
		(successs_array), \
		csalt_arrend(successs_array) \
	)

csalt_store *csalt_resource_decorator_logger_init(csalt_resource *);

/**
 * \brief This decorator implements the store interface around a resource,
 * 	and attempts to initialize and use it only when a method is
 * 	attempted on it.
 *
 * When the resource fails to initialize, csalt_store_read() and
 * csalt_store_write() return -1, csalt_store_size() returns 0 and
 * csalt_store_split() calls the block with a no-op store.
 *
 * Initialization will be attempted on each call to a csalt_store method. Once
 * initialization is successful, it isn't tried again.
 *
 * The recommended use is through csalt_resource_decorator_lazy, which
 * implements the resource interface and automatically deinitializes these
 * decorated store when it is deinitialized. Otherwise, you have to test
 * csalt_decorator_lazy.result and call csalt_resource_deinit()
 * on csalt_decorator_lazy.resource manually.
 *
 * \sa csalt_decorator_lazy()
 */
struct csalt_decorator_lazy {
	const struct csalt_store_interface *vtable;
	csalt_resource *resource;
	csalt_store *result;
};

/**
 * \brief Constructor for a csalt_decorator_lazy, wrapping a resource.
 *
 * Is it a resource decorator? Is it a store decorator? Are viruses life, or
 * not life?
 *
 * \sa csalt_decorator_lazy
 */
struct csalt_decorator_lazy csalt_decorator_lazy(csalt_resource *resource);

ssize_t csalt_decorator_lazy_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
);
ssize_t csalt_decorator_lazy_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
);
ssize_t csalt_decorator_lazy_size(csalt_store *store);
int csalt_decorator_lazy_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
);

/**
 * \brief This decorator decorates a resource and only initializes it when
 * 	an attempt is made to read/write/etc. on the resulting store.
 *
 * csalt_resource_init() never fails for this type of decorator. It returns
 * a csalt_decorator_lazy, which, when csalt_store_read(),
 * csalt_store_write(), csalt_store_size() or csalt_store_split() are called
 * on it, attempts to initialize the real resource before performing the
 * operation.
 *
 * If the real resource fails to initialize at csalt_store_read() or
 * csalt_store_write(), those functions return -1. If the real resource fails
 * to initialize on csalt_store_size(), \c (ssize_t)-1 is returned. If the real
 * resource fails to initialize on csalt_store_split(),
 * csalt_store_null_implementation is passed to the callback.
 *
 * \sa csalt_resource_decorator_lazy()
 * \sa csalt_decorator_lazy
 */
struct csalt_resource_decorator_lazy {
	const struct csalt_dynamic_resource_interface *vtable;
	struct csalt_decorator_lazy decorator;
};

/**
 * \brief Constructor for a csalt_resource_decorator_lazy.
 */
struct csalt_resource_decorator_lazy csalt_resource_decorator_lazy(
	csalt_resource *resource
);

csalt_store *csalt_resource_decorator_lazy_init(csalt_resource *resource);
void csalt_resource_decorator_lazy_deinit(csalt_resource *resource);

/**
 * \brief Decorates a store with a resource which manages a mutex for the
 * 	store.
 *
 * init() and deinit() simply initialize and clean up the mutex.
 *
 * \sa csalt_decorator_mutex()
 */
struct csalt_decorator_mutex {
	const struct csalt_dynamic_resource_interface *vtable;
	struct csalt_store_decorator_mutex decorator;
};

/**
 * \brief Constructor for a csalt_decorator_mutex.
 */
struct csalt_decorator_mutex csalt_decorator_mutex(csalt_store *store, csalt_mutex *mutex);

csalt_store *csalt_decorator_mutex_init(csalt_resource *resource);
void csalt_decorator_mutex_deinit(csalt_resource *resource);

#endif // DECORATORRESOURCES_H
