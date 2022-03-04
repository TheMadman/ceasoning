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

#include "csalt/decoratorresources.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

csalt_store *csalt_resource_decorator_init(csalt_resource *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_init(decorator->child);
}

void csalt_resource_decorator_deinit(csalt_resource *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_deinit((void *)decorator->child);
}

typedef struct csalt_resource_decorator_logger csalt_logger;

struct csalt_resource_interface logger_implementation = {
	csalt_resource_decorator_logger_init,
	csalt_resource_decorator_deinit,
};

struct csalt_store_interface logger_initialized_implementation = {
	csalt_store_decorator_logger_read,
	csalt_store_decorator_logger_write,
	csalt_store_decorator_size,
	csalt_store_decorator_split,
};

struct csalt_resource_decorator_logger csalt_resource_decorator_logger_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *errors_begin,
	const struct csalt_store_log_message *errors_end,
	const struct csalt_store_log_message *successes_begin,
	const struct csalt_store_log_message *successes_end,
	const struct csalt_store_log_message *zero_bytes_begin,
	const struct csalt_store_log_message *zero_bytes_end
)
{
	csalt_logger result = {
		{
			&logger_implementation,
			child,
	       	},
		{
			{
				&logger_initialized_implementation,
				0,
			},
			file_descriptor,
			{
				{ errors_begin, errors_end },
				{ successes_begin, successes_end },
				{ zero_bytes_begin, zero_bytes_end },
			},
		},
	};

	return result;
}

struct csalt_resource_decorator_logger csalt_resource_decorator_logger_error_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *messages_begin,
	const struct csalt_store_log_message *messages_end
) {
	return csalt_resource_decorator_logger_bounds(
		child,
		file_descriptor,
		messages_begin,
		messages_end,
		0,
		0,
		0,
		0
	);
}

struct csalt_resource_decorator_logger csalt_resource_decorator_logger_success_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *messages_begin,
	const struct csalt_store_log_message *messages_end
) {
	return csalt_resource_decorator_logger_bounds(
		child,
		file_descriptor,
		0,
		0,
		messages_begin,
		messages_end,
		0,
		0
	);
}

struct csalt_resource_decorator_logger csalt_resource_decorator_logger_zero_bytes_bounds(
	csalt_resource *child,
	int file_descriptor,
	const struct csalt_store_log_message *messages_begin,
	const struct csalt_store_log_message *messages_end
) {
	return csalt_resource_decorator_logger_bounds(
		child,
		file_descriptor,
		0,
		0,
		0,
		0,
		messages_begin,
		messages_end
	);
}

csalt_store *csalt_resource_decorator_logger_init(csalt_resource *resource)
{
	int old_errno = errno;
	errno = 0;
	csalt_logger *logger = (csalt_logger *)resource;
	csalt_store *initialized_child = csalt_resource_init(logger->decorator.child);
	if (!initialized_child) {
		const char *message = csalt_store_log_message_list_get_message(
			&logger->store_logger.message_lists[0],
			csalt_resource_init
		);
		if (!message)
			goto ERROR_NO_MESSAGE;


		char error_message[1024] = { 0 };
		int strerror_return = strerror_r(errno, error_message, sizeof(error_message));
		if (!strerror_return)
			dprintf(
				logger->store_logger.file_descriptor,
					"%s: csalt_resource_init(%p) -> 0\n"
					"errno: %d\n"
					"Error message: %s",
				message,
				logger->decorator.child,
				errno,
				error_message
			);
		else
			dprintf(
				logger->store_logger.file_descriptor,
					"%s: csalt_resource_init(%p) -> 0\n"
					"errno: %d",
				message,
				logger->decorator.child,
				errno
			);

		goto ERROR_WITH_MESSAGE;
	}

	const char *message = csalt_store_log_message_list_get_message(
		&logger->store_logger.message_lists[1],
		csalt_resource_init
	);
	if (message)
		dprintf(
			logger->store_logger.file_descriptor,
			"%s: csalt_resource_init(%p) -> %p",
			message,
			logger->decorator.child,
			initialized_child
		);

	logger->store_logger.decorator.child = (csalt_store *)initialized_child;

	errno = old_errno;
	return (csalt_store *)&logger->store_logger;

ERROR_WITH_MESSAGE:
ERROR_NO_MESSAGE:
	errno = old_errno;
	return 0;
}

static struct csalt_store_interface csalt_decorator_lazy_implementation = {
	csalt_decorator_lazy_read,
	csalt_decorator_lazy_write,
	csalt_decorator_lazy_size,
	csalt_decorator_lazy_split,
};

struct csalt_decorator_lazy csalt_decorator_lazy(csalt_resource *resource)
{
	struct csalt_decorator_lazy result = {
		&csalt_decorator_lazy_implementation,
		resource,
		0,
	};

	return result;
}

ssize_t csalt_decorator_lazy_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
)
{
	struct csalt_decorator_lazy *lazy = (void *)store;
	if (!lazy->result)
		lazy->result = csalt_resource_init(lazy->resource);

	if (!lazy->result)
		return -1;
	return csalt_store_read(lazy->result, buffer, amount);
}

ssize_t csalt_decorator_lazy_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
)
{
	struct csalt_decorator_lazy *lazy = (void *)store;
	if (!lazy->result)
		lazy->result = csalt_resource_init(lazy->resource);

	if (!lazy->result)
		return -1;
	return csalt_store_write(lazy->result, buffer, amount);
}

ssize_t csalt_decorator_lazy_size(csalt_store *store)
{
	struct csalt_decorator_lazy *lazy = (void *)store;
	if (!lazy->result)
		lazy->result = csalt_resource_init(lazy->resource);

	if (!lazy->result)
		return -1;
	return csalt_store_size(lazy->result);
}

int csalt_decorator_lazy_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
)
{
	struct csalt_decorator_lazy *lazy = (void *)store;
	if (!lazy->result)
		lazy->result = csalt_resource_init(lazy->resource);

	if (!lazy->result)
		return block(csalt_store_null, param);
	return csalt_store_split(lazy->result, begin, end, block, param);
}

static struct csalt_resource_interface
	csalt_resource_decorator_lazy_implementation = {
	csalt_resource_decorator_lazy_init,
	csalt_resource_decorator_lazy_deinit,
};

struct csalt_resource_decorator_lazy csalt_resource_decorator_lazy(
	csalt_resource *resource
)
{
	struct csalt_resource_decorator_lazy result = {
		&csalt_resource_decorator_lazy_implementation,
		csalt_decorator_lazy(resource),
	};
	return result;
}

csalt_store *csalt_resource_decorator_lazy_init(csalt_resource *resource)
{
	struct csalt_resource_decorator_lazy *lazy = (void *)resource;
	return csalt_store(&lazy->decorator);
}

void csalt_resource_decorator_lazy_deinit(csalt_resource *resource)
{
	struct csalt_resource_decorator_lazy *lazy = (void *)resource;
	if (lazy->decorator.result) {
		csalt_resource_deinit(lazy->decorator.resource);
		lazy->decorator.result = 0;
	}
}

struct csalt_resource_interface csalt_decorator_mutex_implementation = {
	csalt_decorator_mutex_init,
	csalt_decorator_mutex_deinit,
};

struct csalt_decorator_mutex csalt_decorator_mutex(
	csalt_store *store,
	csalt_mutex *mutex
)
{
	return (struct csalt_decorator_mutex){
		&csalt_decorator_mutex_implementation,
		csalt_store_decorator_mutex(store, mutex),
	};
}

csalt_store *csalt_decorator_mutex_init(csalt_resource *resource)
{
	struct csalt_decorator_mutex
		*mutex = (struct csalt_decorator_mutex*)resource;

	// pthread doesn't error... might change this with new platforms
	csalt_mutex_init(mutex->decorator.mutex, 0);
	return (csalt_store *)&mutex->decorator;
}

void csalt_decorator_mutex_deinit(csalt_resource *resource)
{
	struct csalt_decorator_mutex
		*mutex = (struct csalt_decorator_mutex*)resource;

	// more pthread shenanigans...
	while(csalt_mutex_deinit(mutex->decorator.mutex))
		;
}

