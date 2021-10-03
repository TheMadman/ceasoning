#ifndef DECORATORRESOURCES_H
#define DECORATORRESOURCES_H

#include "baseresources.h"
#include "decoratorstores.h"
#include "util.h"

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
	const struct csalt_resource_interface *vtable;
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
	arrend(errors_array), \
	(successes_array), \
	arrend(successes_array), \
	(zero_bytes_array), \
	arrend(zero_bytes_array) \
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
		arrend(errors_array) \
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
		arrend(successs_array) \
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
		arrend(successs_array) \
	)

csalt_store *csalt_resource_decorator_logger_init(csalt_resource *);

#endif // DECORATORRESOURCES_H
