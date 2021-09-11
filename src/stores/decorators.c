#include "csalt/decoratorstores.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_TYPE_ERROR 0
#define LOG_TYPE_SUCCESS 1
#define LOG_TYPE_ZERO_BYTES 2

ssize_t csalt_store_decorator_read(
	const csalt_store *store,
	void *buffer,
	size_t size
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_read(decorator->child, buffer, size);
}

ssize_t csalt_store_decorator_write(
	csalt_store *store,
	const void *buffer,
	size_t size
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_write(decorator->child, buffer, size);
}

int csalt_store_decorator_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
) {
	struct csalt_store_decorator *decorator = (void *)store;
	return csalt_store_split(decorator->child, begin, end, block, data);
}

size_t csalt_store_decorator_size(const csalt_store *store)
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

static const char *message_for_function(const csalt_logger *logger, int log_type, void *function)
{
	for (
		const struct csalt_store_log_message
			*current = logger->message_lists[log_type].begin,
			*end = logger->message_lists[log_type].end;
		current < end;
		current++
	) {
		if (current->function == function)
			return current->message;
	}

	return 0;
}

#define READ_WRITE_FORMAT_STR "%s: %s(%p, %p, %lu) -> %ld\n"

ssize_t csalt_store_decorator_logger_read(const csalt_store *store, void *buffer, size_t bytes)
{
	csalt_logger *logger = (csalt_logger *)store;
	csalt_store *decorated = logger->decorator.child;
	ssize_t result = csalt_store_read(decorated, buffer, bytes);

	if (result < 0) {
		const char *custom_message = message_for_function(logger, LOG_TYPE_ERROR, csalt_store_read);
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
				decorated,
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
				decorated,
				buffer,
				bytes,
				result,
				errnum,
				errbuf
			);
		}
	} else if (result > 0) {
		const char *custom_message = message_for_function(logger, LOG_TYPE_SUCCESS, csalt_store_read);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			"csalt_store_read",
			custom_message,
			decorated,
			buffer,
			bytes,
			result
		);
	} else {
		const char *custom_message = message_for_function(logger, LOG_TYPE_ZERO_BYTES, csalt_store_read);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			"csalt_store_read",
			custom_message,
			decorated,
			buffer,
			bytes,
			result
		);
	}

	return result;
}

ssize_t csalt_store_decorator_logger_write(csalt_store *store, const void *buffer, size_t bytes)
{
	csalt_logger *logger = (csalt_logger *)store;
	csalt_store *decorated = logger->decorator.child;
	ssize_t result = csalt_store_write(decorated, buffer, bytes);

	if (result < 0) {
		const char *custom_message = message_for_function(logger, LOG_TYPE_ERROR, csalt_store_write);
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
				decorated,
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
				decorated,
				buffer,
				bytes,
				result,
				errnum,
				errbuf
			);
		}
	} else if (result > 0) {
		const char *custom_message = message_for_function(logger, LOG_TYPE_SUCCESS, csalt_store_write);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_write",
			decorated,
			buffer,
			bytes,
			result
		);
	} else {
		const char *custom_message = message_for_function(logger, LOG_TYPE_ZERO_BYTES, csalt_store_write);
		if (!custom_message)
			return result;

		dprintf(
			logger->file_descriptor,
			READ_WRITE_FORMAT_STR,
			custom_message,
			"csalt_store_write",
			decorated,
			buffer,
			bytes,
			result
		);
	}

	return result;
}

#define SPLIT_FORMAT_STR "%s: csalt_store_split(%p, %ld, %ld, %p, %p) -> %d"

