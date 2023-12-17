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

#ifndef CSALT_STORES_LOGGER_H
#define CSALT_STORES_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 * \brief This module provides a logging decorator for stores, and related
 * 	data types.
 */

#include "decorator.h"
#include "../util.h"

/**
 * \brief A tuple type used by csalt_store_decorator_logger
 * 	to provide log messages based on the return codes of
 * 	the given functions.
 */
struct csalt_store_log_message {
	/**
	 * \brief Indicates which function should be logged. Must be
	 * one of csalt_store_read or csalt_store_write.
	 */
	void (*function)(void);

	/**
	 * \brief  A useful message for identifying the store. The function
	 * name, passed parameters and return value will automatically
	 * be included in the log.
	 */
	const char *message;
};

/**
 * \public \memberof csalt_store_log_message
 * \brief Convenience constructor for csalt_store_log_message%s.
 *
 * \param fn The function to log (e.g. csalt_store_read, csalt_store_write,
 * 	csalt_store_resize)
 * \param message A useful message for identifying the store.
 *
 * \returns A constructed csalt_store_log_message.
 */
#define csalt_store_log_message(fn, message) \
	((struct csalt_store_log_message){ (void (*)(void))(fn), (message) })

/**
 * \brief This type decorates a store, providing customizable logging
 * 	output.
 *
 * When this store is split, the resulting store is also wrapped in
 * a logger.
 */
struct csalt_store_logger {
	struct csalt_store_decorator parent;

	/**
	 * \brief The output to log to.
	 */
	csalt_static_store *output;

	struct csalt_array
		/**
		 * \brief A csalt_array of csalt_store_log_message%s
		 * 	for error cases.
		 */
		error,

		/**
		 * \brief A csalt_array of csalt_store_log_message%s
		 * 	for success cases.
		 */
		success,

		/**
		 * \brief A csalt_array of csalt_store_log_message%s
		 * 	for read/write calls that return less than the
		 * 	requested bytes.
		 */
		partial_success;
};

ssize_t csalt_store_logger_read(csalt_static_store *, void *, ssize_t);
ssize_t csalt_store_logger_write(csalt_static_store *, const void *, ssize_t);
int csalt_store_logger_split(
	csalt_static_store *,
	ssize_t,
	ssize_t,
	csalt_static_store_block_fn *,
	void *
);
ssize_t csalt_store_logger_resize(csalt_store *, ssize_t);

/**
 * \public \memberof csalt_store_logger
 * \brief Constructcs a csalt_store_logger from the given arrays.
 *
 * \param decorated The store to decorate.
 * \param output The output to log to.
 * \param error Error cases to log. If an empty array is provided,
 * 	no error cases are logged.
 * \param success Success cases to log. If an empty array is provided,
 * 	no success cases are logged.
 * \param partial_success Partially completed read/writes to log.
 * 	If an empty array is provided, no partial successes are logged.
 *
 * \returns The resulting logging store.
 */
struct csalt_store_logger csalt_store_logger_arrays(
	csalt_store *decorated,
	csalt_static_store *output,
	struct csalt_array error,
	struct csalt_array success,
	struct csalt_array partial_success);

/**
 * \public \memberof csalt_store_logger
 * \brief Takes an output store and a C array of csalt_store_log_message%s
 * 	and constructs a new csalt_store_logger.
 */
#define csalt_store_logger_error(decorated, output, error_array) \
	csalt_store_logger_arrays( \
		(decorated), \
		(output), \
		csalt_array(error_array), \
		(struct csalt_array) { 0 }, \
		(struct csalt_array) { 0 })

/**
 * \public \memberof csalt_store_logger
 * \brief Takes an output store and a C array of csalt_store_log_message%s
 * 	and constructs a new csalt_store_logger.
 */
#define csalt_store_logger_success(decorated, output, success_array) \
	csalt_store_logger_arrays( \
		(decorated), \
		(output), \
		(struct csalt_array) { 0 }, \
		csalt_array(success_array), \
		(struct csalt_array) { 0 })

/**
 * \public \memberof csalt_store_logger
 * \brief Takes an output store and a C array of csalt_store_log_message%s
 * 	and constructs a new csalt_store_logger.
 */
#define csalt_store_logger_partial_success(decorated, output, ps_array) \
	csalt_store_logger_arrays( \
		(decorated), \
		(output), \
		(struct csalt_array) { 0 }, \
		(struct csalt_array) { 0 }, \
		csalt_array(ps_array))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_STORES_LOGGER_H
