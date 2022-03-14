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

#include "csalt/decoratorstores.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TYPE_ERROR 0
#define LOG_TYPE_SUCCESS 1
#define LOG_TYPE_ZERO_BYTES 2

ssize_t csalt_store_decorator_read(
	csalt_store *store,
	void *buffer,
	ssize_t size
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_read(decorator->child, buffer, size);
}

ssize_t csalt_store_decorator_write(
	csalt_store *store,
	const void *buffer,
	ssize_t size
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_write(decorator->child, buffer, size);
}

int csalt_store_decorator_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_split(decorator->child, begin, end, block, data);
}

ssize_t csalt_store_decorator_size(csalt_store *store)
{
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_size(decorator->child);
}

static struct csalt_store_interface csalt_store_decorator_logger_implementation = {
	csalt_store_decorator_logger_read,
	csalt_store_decorator_logger_write,
	csalt_store_decorator_size,
	csalt_store_decorator_split,
};

struct csalt_store_decorator_logger csalt_store_decorator_logger_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
)
{
	struct csalt_store_decorator_logger result = {
		{
			&csalt_store_decorator_logger_implementation,
			store,
		},
		file_descriptor,
		{
			{
				errors_begin,
				errors_end,
			},
			{
				successes_begin,
				successes_end,
			},
			{
				zero_bytes_begin,
				zero_bytes_end,
			},
		},
	};

	return result;
}

struct csalt_store_decorator_logger csalt_store_decorator_logger_error_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end
)
{
	return csalt_store_decorator_logger_bounds(
		store,
		file_descriptor,
		errors_begin,
		errors_end,
		0,
		0,
		0,
		0
	);
}


struct csalt_store_decorator_logger csalt_store_decorator_logger_success_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end
)
{
	return csalt_store_decorator_logger_bounds(
		store,
		file_descriptor,
		0,
		0,
		successes_begin,
		successes_end,
		0,
		0
	);
}

struct csalt_store_decorator_logger csalt_store_decorator_logger_zero_bytes_bounds(
	csalt_store *store,
	int file_descriptor,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
)
{
	return csalt_store_decorator_logger_bounds(
		store,
		file_descriptor,
		0,
		0,
		0,
		0,
		zero_bytes_begin,
		zero_bytes_end
	);
}

typedef struct csalt_store_decorator_logger csalt_logger;

const char *csalt_store_log_message_list_get_message(
	const struct csalt_store_log_message_list *list,
	void *function
)
{
	if (!list->begin)
		return 0;

	for (
		const struct csalt_store_log_message
			*current = list->begin,
			*end = list->end;
		current < end;
		current++
	) {
		if (current->function == function)
			return current->message;
	}

	return 0;
}

static const char *message_for_function(
	const csalt_logger *logger,
	int log_type,
	void *function
)
{
	return csalt_store_log_message_list_get_message(
		&logger->message_lists[log_type],
		function
	);
}

#define READ_WRITE_FORMAT_STR "%s: %s(%p, %p, %lu) -> %ld\n"

ssize_t csalt_store_decorator_logger_read(csalt_store *store, void *buffer, ssize_t bytes)
{
	csalt_logger *logger = (csalt_logger *)store;
	csalt_store *decorated = logger->decorator.child;
	ssize_t result = csalt_store_read(decorated, buffer, bytes);

	if (result < 0) {
		const char *custom_message = message_for_function(
			logger,
			LOG_TYPE_ERROR,
			(void *)csalt_store_read
		);

		if (!custom_message)
			return result;

		char errbuf[512] = { 0 };
		int errnum = errno;
		int strerror_result = strerror_r(errnum, errbuf, sizeof(errbuf));
		if (strerror_result) {
			dprintf(
				logger->file_descriptor,
					READ_WRITE_FORMAT_STR
					"errno: %d\n"
					"Could not retrieve error string",
				custom_message,
				"csalt_store_read",
				(void *)decorated,
				buffer,
				bytes,
				result,
				errnum
			);
		} else {
			dprintf(
				logger->file_descriptor,
					READ_WRITE_FORMAT_STR
					"errno: %d\n"
					"Error message: %s\n",
				custom_message,
				"csalt_store_read",
				(void *)decorated,
				buffer,
				bytes,
				result,
				errnum,
				errbuf
			);
		}
	} else if (result > 0) {
		const char
			*custom_message = message_for_function(
				logger,
				LOG_TYPE_SUCCESS,
				(void *)csalt_store_read
			);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_read",
			(void *)decorated,
			buffer,
			bytes,
			result
		);
	} else {
		const char
			*custom_message = message_for_function(
				logger,
				LOG_TYPE_ZERO_BYTES,
				(void *)csalt_store_read
			);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_read",
			(void *)decorated,
			buffer,
			bytes,
			result
		);
	}

	return result;
}

ssize_t csalt_store_decorator_logger_write(csalt_store *store, const void *buffer, ssize_t bytes)
{
	csalt_logger *logger = (csalt_logger *)store;
	csalt_store *decorated = logger->decorator.child;
	ssize_t result = csalt_store_write(decorated, buffer, bytes);

	if (result < 0) {
		const char
			*custom_message = message_for_function(
				logger,
				LOG_TYPE_ERROR,
				(void *)csalt_store_write
			);
		if (!custom_message)
			return result;

		char errbuf[512] = { 0 };
		int errnum = errno;
		int strerror_result = strerror_r(errnum, errbuf, sizeof(errbuf));
		if (strerror_result) {
			dprintf(
				logger->file_descriptor,
					READ_WRITE_FORMAT_STR
					"errno: %d\n"
					"Could not retrieve error string",
				custom_message,
				"csalt_store_write",
				(void *)decorated,
				buffer,
				bytes,
				result,
				errnum
			);
		} else {
			dprintf(
				logger->file_descriptor,
					READ_WRITE_FORMAT_STR
					"errno: %d\n"
					"Error message: %s\n",
				custom_message,
				"csalt_store_write",
				(void *)decorated,
				buffer,
				bytes,
				result,
				errnum,
				errbuf
			);
		}
	} else if (result > 0) {
		const char
			*custom_message = message_for_function(
				logger,
				LOG_TYPE_SUCCESS,
				(void *)csalt_store_write
			);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_write",
			(void *)decorated,
			buffer,
			bytes,
			result
		);
	} else {
		const char
			*custom_message = message_for_function(
				logger,
				LOG_TYPE_ZERO_BYTES,
				(void *)csalt_store_write
			);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_write",
			(void *)decorated,
			buffer,
			bytes,
			result
		);
	}

	return result;
}

ssize_t csalt_store_decorator_mutex_read(
	csalt_store *store,
	void *buffer,
	ssize_t size
)
{
	struct csalt_store_decorator_mutex *mutex = (void *)store;

	int try_lock = csalt_mutex_lock(mutex->mutex);
	if (try_lock)
		return -1;
	ssize_t result = csalt_store_read(mutex->decorator.child, buffer, size);
	csalt_mutex_unlock(mutex->mutex);
	return result;
}

ssize_t csalt_store_decorator_mutex_write(
	csalt_store *store,
	const void *buffer,
	ssize_t size
)
{
	struct csalt_store_decorator_mutex *mutex = (void *)store;

	int try_lock = csalt_mutex_lock(mutex->mutex);
	if (try_lock)
		return -1;

	ssize_t result = csalt_store_write(
		mutex->decorator.child,
		buffer,
		size
	);
	csalt_mutex_unlock(mutex->mutex);
	return result;
}

ssize_t csalt_store_decorator_mutex_size(csalt_store *store)
{
	struct csalt_store_decorator_mutex *mutex = (void *)store;

	return csalt_store_size(mutex->decorator.child);
}

struct decorator_mutex_split_params {
	csalt_store_block_fn *block;
	void *param;
	csalt_mutex *mutex;
};

static int mutex_receive_split(csalt_store *store, void *param)
{
	struct decorator_mutex_split_params *original_params = param;
	csalt_mutex_unlock(original_params->mutex);

	struct csalt_store_decorator_mutex
		result = csalt_store_decorator_mutex(store, original_params->mutex);

	return original_params->block(csalt_store(&result), original_params->param);
}

int csalt_store_decorator_mutex_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	struct csalt_store_decorator_mutex *mutex = (void *)store;
	struct decorator_mutex_split_params original_params = {
		block,
		param,
		mutex->mutex,
	};

	int try_lock = csalt_mutex_lock(mutex->mutex);
	if (try_lock)
		return -1;

	return csalt_store_split(
		mutex->decorator.child,
		begin,
		end,
		mutex_receive_split,
		&original_params
	);
}

struct csalt_store_interface csalt_store_decorator_mutex_implementation = {
	csalt_store_decorator_mutex_read,
	csalt_store_decorator_mutex_write,
	csalt_store_decorator_mutex_size,
	csalt_store_decorator_mutex_split,
};

struct csalt_store_decorator_mutex csalt_store_decorator_mutex(
	csalt_store *store,
	csalt_mutex *mutex
)
{
	struct csalt_store_decorator_mutex result = {
		{
			&csalt_store_decorator_mutex_implementation,
			store,
		},
		mutex
	};
	return result;
}

ssize_t csalt_store_decorator_rwlock_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
)
{
	struct csalt_store_decorator_rwlock *lock = (void *)store;
	int try_lock = csalt_rwlock_rdlock(lock->rwlock);
	if (try_lock)
		return -1;
	ssize_t result = csalt_store_read(lock->decorator.child, buffer, amount);
	csalt_rwlock_unlock(lock->rwlock);
	return result;
}

ssize_t csalt_store_decorator_rwlock_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
)
{
	struct csalt_store_decorator_rwlock *lock = (void *)store;
	int try_lock = csalt_rwlock_wrlock(lock->rwlock);
	if (try_lock)
		return -1;
	ssize_t result = csalt_store_write(lock->decorator.child, buffer, amount);
	csalt_rwlock_unlock(lock->rwlock);
	return result;
}

ssize_t csalt_store_decorator_rwlock_size(csalt_store *store)
{
	struct csalt_store_decorator_rwlock *lock = (void *)store;

	return csalt_store_size(lock->decorator.child);
}

struct decorator_rwlock_split_params {
	csalt_store_block_fn *block;
	void *param;
	csalt_rwlock *rwlock;
};

static int rwlock_receive_split(csalt_store *store, void *param)
{
	struct decorator_rwlock_split_params *original_params = param;
	csalt_rwlock_unlock(original_params->rwlock);

	struct csalt_store_decorator_rwlock
		result = csalt_store_decorator_rwlock(
			store,
			original_params->rwlock
		);

	return original_params->block(
		csalt_store(&result),
		original_params->param
	);
}

int csalt_store_decorator_rwlock_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	struct csalt_store_decorator_rwlock *lock = (void *)store;

	struct decorator_rwlock_split_params original_params = {
		block,
		param,
		lock->rwlock,
	};

	int try_lock = csalt_rwlock_wrlock(lock->rwlock);
	if (try_lock)
		return -1;

	return csalt_store_split(
		lock->decorator.child,
		begin,
		end,
		rwlock_receive_split,
		&original_params
	);
}

static struct csalt_store_interface rwlock_implementation = {
	csalt_store_decorator_rwlock_read,
	csalt_store_decorator_rwlock_write,
	csalt_store_decorator_rwlock_size,
	csalt_store_decorator_rwlock_split,
};

struct csalt_store_decorator_rwlock csalt_store_decorator_rwlock(
	csalt_store *store,
	csalt_rwlock *rwlock
)
{
	struct csalt_store_decorator_rwlock result = {
		{
			&rwlock_implementation,
			store,
		},
		rwlock,
	};

	return result;
}

