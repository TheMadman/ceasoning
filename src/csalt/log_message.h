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

#ifndef CSALT_LOG_MESSAGE_H
#define CSALT_LOG_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util.h"

/**
 * \file
 * \brief Provides the csalt_log_message struct, as well as convenience
 * 	functions for looking up the log_message struct in an array.
 */

/**
 * \brief A tuple type used by csalt_store_decorator_logger
 * 	to provide log messages based on the return codes of
 * 	the given functions.
 */
struct csalt_log_message {
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
 * \public \memberof csalt_log_message
 * \brief Convenience constructor for csalt_log_message%s.
 *
 * \param fn The function to log (e.g. csalt_store_read, csalt_store_write,
 * 	csalt_store_resize)
 * \param message A useful message for identifying the store.
 *
 * \returns A constructed csalt_log_message.
 */
#define csalt_log_message(fn, message) \
	((struct csalt_log_message){ (void (*)(void))(fn), (message) })

/**
 * \brief Function for searching an array for the log message of
 * 	a given function.
 *
 * \param array An array of csalt_log_message%s to search
 * \param function The function to search the array for
 *
 * \returns The log message if found, or NULL if no message for
 * 	this function.
 */
const char *csalt_log_message_get(
	struct csalt_array array,
	void (*function)(void)
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_LOG_MESSAGE_H
