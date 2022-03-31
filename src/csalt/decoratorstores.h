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

#ifndef DECORATORSTORES_H
#define DECORATORSTORES_H

#include <csalt/platform/init.h>

#include "basestores.h"
#include <csalt/platform/threads.h>

/**
 * \file
 *
 * \brief This file is responsible for providing decorator
 * functions around csalt_store%s.
 *
 * Decorators take another csalt_store in their
 * constructor and wrap its functionality. For example,
 * a profiling decorator may implement a custom read()
 * or write() function, taking the same arguments. It would
 * sample a timestamp at the beginning of the function, call
 * the decorated store's read() or write() function with
 * the parameters given, sample a timestamp after they
 * have finished, calculate and store (or log) the duration
 * before finally returning the result of the read() or
 * write() call.
 *
 * The decorator itself, csalt_store_decorator, can't be
 * instanced. You are intended to include it at the beginning
 * of implementations and configure it yourself. The decorator_*
 * functions are provided as useful defaults for decorators,
 * and they take a csalt_store_decorator and simply forward
 * the call.
 *
 * This file also provides some default store decorators.
 */

/**
 * \brief A default read decorator, which takes a 
 * csalt_store_decorator and forwards the call.
 */
ssize_t csalt_store_decorator_read(
	csalt_store *store,
	void *buffer,
	ssize_t size
);

/**
 * \brief A default write decorator, which takes
 * a csalt_store_decorator and forwards the call.
 */
ssize_t csalt_store_decorator_write(
	csalt_store *store,
	const void *buffer,
	ssize_t size
);

/**
 * \brief A default split decorator, which takes
 * a csalt_store_decorator and forwards the call.
 */
int csalt_store_decorator_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief A default size decorator, which takes
 * a csalt_store_decorator and forwards the call.
 */
ssize_t csalt_store_decorator_size(csalt_store *store);

struct csalt_store_decorator {
	const struct csalt_store_interface *vtable;
	csalt_store *child;
};

/**
 * \brief A tuple type used by csalt_store_decorator_logger
 * to provide log messages based on the return codes of the given functions.
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

#define csalt_store_log_message(fn, message) ((struct csalt_store_log_message){ (void (*)(void))(fn), (message) })

struct csalt_store_log_message_list {
	const struct csalt_store_log_message *begin;
	const struct csalt_store_log_message *end;
};

#define csalt_store_log_message_list(array) ((struct csalt_store_log_message_array){ (array), arrend(array) })

/**
 * \brief Searches a list for a given function and returns
 * 	the associated message, or 0 if none is found.
 */
const char *csalt_store_log_message_list_get_message(
	const struct csalt_store_log_message_list *list,
	void (*function)(void)
);

/**
 * \brief This object is responsible for wrapping a csalt_store's
 * functions, checking return values and logging messages.
 *
 * It logs for three different cases - an error occurred; the function
 * call was successful; and, as a special case, a read/write call
 * was successful but returned 0 bytes.
 *
 * Besides the given csalt_store_log_message messages, the function
 * name, passed parameters and return values are printed. In the
 * case of errors, the value of errno and its corresponding error string
 * are also printed.
 *
 * An example which logs errors to stderr:
 * \code
 * 	// csalt_store *file;
 *	struct csalt_store_log_message messages[] = {
 *		{ csalt_store_read, "tempfile" },
 *		{ csalt_store_write, "tempfile" },
 *	};
 *	struct csalt_store_decorator_logger logger =
 *		csalt_store_decorator_logger_error_array(file, stderr, messages);
 *	
 *	const char data[] = "Hello, World!";
 *	ssize_t write_result = csalt_store_write((csalt_store *)&logger, data, sizeof(data));
 *	if (write_result < 0)
 *		// Message was already logged to stderr for us, just return
 *		return EXIT_FAILURE;
 *
 *	char read_data[sizeof(data)] = { 0 };
 *	ssize_t read_result = csalt_store_read((csalt_store *)&logger, read_data, sizeof(read_data));
 *	if (read_result < 0)
 *		return EXIT_FAILURE;
 * \endcode
 *
 * \see csalt_store_decorator_logger_error_array
 * \see csalt_store_decorator_logger_success_array
 * \see csalt_store_decorator_logger_zero_bytes_array
 * \see csalt_store_decorator_logger_arrays
 */
struct csalt_store_decorator_logger {
	struct csalt_store_decorator decorator;
	int file_descriptor;
	struct csalt_store_log_message_list message_lists[3];
};

/**
 * \brief Constructore for csalt_store_decorator_logger which takes
 * the beginning and end boundaries for log_message arrays.
 *
 * As well as a csalt_store to decorate and the log message lists,
 * this function takes the file descriptor to log output to.
 */
struct csalt_store_decorator_logger csalt_store_decorator_logger_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
);

/**
 * \brief Convenience macro taking array arguments directly
 */
#define csalt_store_decorator_logger(store, fd, errors, successes, zero_bytes) \
	(csalt_store_decorator_logger_bounds( \
		store, \
		fd, \
		(errors), \
		(arrend(errors)), \
		(successes), \
		(arrend(successes)), \
		(zero_bytes), \
		(arrend(zero_bytes)) \
	))

/**
 * \brief Convenience constructor for creating a logger which only
 * logs error statuses.
 */
struct csalt_store_decorator_logger csalt_store_decorator_logger_error_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end
);

/**
 * \brief Convenience wrapper taking an array argument directly
 */
#define csalt_store_decorator_logger_errors(store, fd, array) \
	(csalt_store_decorator_logger_error_bounds(store, fd, (array), (arrend(array))))


/**
 * \brief Convenience constructor for creating a logger which only logs
 * success statuses.
 */
struct csalt_store_decorator_logger csalt_store_decorator_logger_success_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end
);

/**
 * \brief Convenience wrapper taking an array argument directly
 */
#define csalt_store_decorator_logger_successes(store, fd, array) \
	(csalt_store_decorator_logger_success_bounds(store, fd, (array), (arrend(array))))

/**
 * \brief Convenience constructor for creating a logger which only
 * logs zero-byte read/write calls.
 */
struct csalt_store_decorator_logger csalt_store_decorator_logger_zero_bytes_bounds(
	csalt_store *store,
	int fild_descriptor,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
);

/**
 * \brief Convenience wrapper taking an array argument directly
 */
#define csalt_store_decorator_logger_zero_bytes(store, fd, array) \
	(csalt_store_decorator_logger_zero_bytes_bounds(store, fd, (array), (arrend(array))))

/**
 * \brief Implementation for logger read function
 */
ssize_t csalt_store_decorator_logger_read(
	csalt_store *store,
	void *buffer,
	ssize_t bytes
);

/**
 * \brief Implementation for logger write function
 */
ssize_t csalt_store_decorator_logger_write(
	csalt_store *store,
	const void *buffer,
	ssize_t bytes
);

/**
 * \brief Converts byte-indexing into array-like-indexing.
 *
 * If your store's data represents an array of fixed-length elements,
 * this decorator scales the traditional "bytes" argument by the size
 * of each element automatically, similarly to the C array index operator
 * square brackets.
 *
 * For example, for an array of `int`s which are 32 bits (4 bytes) long,
 * a csalt_store_read(store, buffer, 1) will attempt to read 4 bytes;
 * a csalt_store_split(store, 3, 5, block, arg) will split beginning at
 * the 3rd `int` (12 bytes from the beginning) and ending at the 5th `int`
 * (20 bytes from the beginning), and so on.
 *
 * csalt_store_size() returns the original store's size divided by the
 * element size.
 *
 * While csalt_store_split() will split the store at the given elements,
 * the resulting store returns to byte-indexing operations: this is to
 * allow using generic csalt_store_block_fn functions. If the resulting
 * store continued to use array-style indexing, the passed block would
 * need to know whether the result were byte- or array-index in order
 * to function correctly.
 *
 * If you want to use array-indexing on the passed store, simply wrap
 * it in an array decorator. This will also allow your function to
 * work correctly for stores which are not array-indexed, making it
 * more flexible.
 *
 * \sa csalt_store_decorator_array()
 * \sa csalt_store_decorator_array_set()
 * \sa csalt_store_decorator_array_get()
 */
struct csalt_store_decorator_array {
	struct csalt_store_decorator decorator;
	ssize_t element_size;
};

/**
 * \brief Constructs a csalt_store_decorator_array.
 *
 * \sa csalt_store_decorator_array
 */
struct csalt_store_decorator_array
	csalt_store_decorator_array(csalt_store *store, ssize_t element_size);

ssize_t csalt_store_decorator_array_read(
	csalt_store *store,
	void *buffer,
	ssize_t bytes
);

ssize_t csalt_store_decorator_array_write(
	csalt_store *store,
	const void *buffer,
	ssize_t bytes
);

ssize_t csalt_store_decorator_array_size(
	csalt_store *store
);

int csalt_store_decorator_array_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

/**
 * \brief Convenience function for writing a single value
 * 	into a given array index.
 *
 * returns 1 on success, 0 on failure.
 */
int csalt_store_decorator_array_set(
	struct csalt_store_decorator_array *array,
	const void *value,
	ssize_t index
);

/**
 * \brief Convenience function for reading a single value
 * 	out of a given array index.
 *
 * returns 1 on success, 0 on failure.
 */
int csalt_store_decorator_array_get(
	struct csalt_store_decorator_array *array,
	void *value,
	ssize_t index
);

/**
 * \brief Provides a means to lock accesses to the store behind a mutex.
 *
 * Use this decorator, or csalt_resource_decorator_mutex, when sharing a store
 * across multiple threads to keep accesses synchronized.
 *
 * In the case that a mutex lock fails when calling csalt_store_size, the
 * function call returns 0. It may also return 0 if the underlying store
 * returns a size of 0.
 *
 * The following functions trigger a lock: csalt_store_read(),
 * csalt_store_write() and csalt_store_split(). csalt_store_size() is
 * unsynchronized, as depending on the value of a csalt_store_size() for
 * a call to another function is still a race condition, unless performed
 * inside a csalt_store_split() block.
 *
 * During a csalt_store_split(), the entire block is performed under mutex
 * lock. Therefore, csalt_store_split() can be used as a transactional
 * interface.
 *
 * This store type can block on calls if other threads have locked the mutex.
 */
struct csalt_store_decorator_mutex {
	struct csalt_store_decorator decorator;
	csalt_mutex *mutex;
};

/**
 * \brief Constructor for a csalt_store_decorator_mutex.
 */
struct csalt_store_decorator_mutex csalt_store_decorator_mutex(
	csalt_store *store,
	csalt_mutex *mutex
);

ssize_t csalt_store_decorator_mutex_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
);

ssize_t csalt_store_decorator_mutex_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
);

ssize_t csalt_store_decorator_mutex_size(csalt_store *store);

int csalt_store_decorator_mutex_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

/**
 * \brief Provides a means to synchronize access to the store via a read/write
 * 	lock.
 *
 * This type performs a read lock for calls to csalt_store_read() and performs
 * a write lock for csalt_store_write() and csalt_store_split().
 *
 * csalt_store_size() is unsynchronized, since calling csalt_store_size() and
 * depending on the result for a call to csalt_store_read() or
 * csalt_store_write() would result in a race condition.
 *
 * Read/write locks allow multiple concurrent readers, but only one writer at
 * any time. Writes block on current readers, and take priority over pending
 * readers. Reads block on current writers and have lower priority than
 * pending writers.
 */
struct csalt_store_decorator_rwlock {
	struct csalt_store_decorator decorator;
	csalt_rwlock *rwlock;
};

/**
 * \brief Constructor for a csalt_store_decorator_rwlock.
 */
struct csalt_store_decorator_rwlock csalt_store_decorator_rwlock(
	csalt_store *store,
	csalt_rwlock *rwlock
);

ssize_t csalt_store_decorator_rwlock_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
);

ssize_t csalt_store_decorator_rwlock_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
);

ssize_t csalt_store_decorator_rwlock_size(csalt_store *store);

int csalt_store_decorator_rwlock_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

#endif //DECORATORSTORES_H
