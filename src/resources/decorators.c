#include "csalt/decoratorresources.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

csalt_resource_initialized *csalt_resource_decorator_init(csalt_resource *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_init(decorator->child);
}

void csalt_resource_decorator_deinit(csalt_resource_initialized *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_deinit((void *)decorator->child);
}

typedef struct csalt_resource_decorator_logger csalt_logger;

struct csalt_resource_interface logger_implementation = {
	csalt_resource_decorator_logger_init,
};

struct csalt_resource_initialized_interface logger_initialized_implementation = {
	{
		csalt_store_decorator_logger_read,
		csalt_store_decorator_logger_write,
		csalt_store_decorator_size,
		csalt_store_decorator_split,
	},
	csalt_resource_decorator_deinit,
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
				&logger_initialized_implementation.parent,
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

csalt_resource_initialized *csalt_resource_decorator_logger_init(csalt_resource *resource)
{
	int old_errno = errno;
	errno = 0;
	csalt_logger *logger = (csalt_logger *)resource;
	csalt_resource_initialized *initialized_child = csalt_resource_init(logger->decorator.child);
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
	return (csalt_resource_initialized *)&logger->store_logger;

ERROR_WITH_MESSAGE:
ERROR_NO_MESSAGE:
	errno = old_errno;
	return 0;
}

