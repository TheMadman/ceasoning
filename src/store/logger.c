#include "csalt/store/logger.h"

#include "csalt/util.h"
#include "csalt/resource/format.h"

#include <stdarg.h>

typedef struct csalt_store_logger logger_t;
typedef struct csalt_log_message message_t;
typedef void void_fn(void);

typedef enum {
	RETURN_ERROR,
	RETURN_SUCCESS,
	RETURN_PARTIAL_SUCCESS,
} RETURN_TYPE;

static RETURN_TYPE get_return_type(ssize_t expected, ssize_t actual)
{
	if (actual < 0)
		return RETURN_ERROR;
	else if (actual == expected)
		return RETURN_SUCCESS;
	else
		return RETURN_PARTIAL_SUCCESS;
}

static const char *get_message_for(
	const logger_t *logger,
	void_fn *fn,
	ssize_t size,
	ssize_t result
)
{
	const struct csalt_array *const message_lists[] = {
		&logger->error,
		&logger->success,
		&logger->partial_success,
	};

	const struct csalt_array *const current = message_lists[get_return_type(size, result)];

	return csalt_log_message_get(*current, fn);
}

static int use_format(csalt_store *store, void *param)
{
	csalt_static_store **output = param;
	struct csalt_progress progress = csalt_progress(csalt_store_size(store));
	while (!csalt_progress_complete(&progress))
		if (csalt_store_transfer(&progress, (void*)store, *output) == -1)
			return -1;
	return 0;
}

ssize_t csalt_store_logger_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
)
{
	logger_t *logger = (logger_t*)store;
	const ssize_t result = csalt_store_read(
		logger->parent.decorated_static,
		buffer,
		size);

	const char *message = get_message_for(
		logger,
		(void_fn*)csalt_store_read,
		size,
		result);

	if (message)
		csalt_use_format(
			use_format,
			&logger->output,
			"%s: csalt_store_read(%p, %p, %ld) -> %ld\n",
			message,
			logger->parent.decorated_static,
			buffer,
			size,
			result);

	return result;
}

ssize_t csalt_store_logger_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
)
{
	logger_t *logger = (logger_t*)store;
	const ssize_t result = csalt_store_write(
		logger->parent.decorated_static,
		buffer,
		size);

	const char *message = get_message_for(
		logger,
		(void_fn*)csalt_store_write,
		size,
		result);

	if (message)
		csalt_use_format(
			use_format,
			&logger->output,
			"%s: csalt_store_write(%p, %p, %ld) -> %ld\n",
			message,
			logger->parent.decorated_static,
			buffer,
			size,
			result);

	return result;
}

static RETURN_TYPE get_resize_return_type(ssize_t original, ssize_t new, ssize_t result)
{
	if (result == original)
		return RETURN_ERROR;
	else if (result == new)
		return RETURN_SUCCESS;
	else
		return RETURN_PARTIAL_SUCCESS;
}

ssize_t csalt_store_logger_resize(
	csalt_store *store,
	ssize_t new_size
)
{
	logger_t *logger = (logger_t*)store;
	const ssize_t original = csalt_store_size(logger->parent.decorated);

	const ssize_t result = csalt_store_resize(
		logger->parent.decorated,
		new_size);

	const struct csalt_array *const message_lists[] = {
		&logger->error,
		&logger->success,
		&logger->partial_success,
	};

	const RETURN_TYPE return_type = get_resize_return_type(original, new_size, result);
	const struct csalt_array *const list = message_lists[return_type];

	const char *message = csalt_log_message_get(*list, (void_fn*)csalt_store_resize);

	if (message)
		csalt_use_format(
			use_format,
			&logger->output,
			"%s: csalt_store_resize(%p, %ld) -> %ld\n",
			message,
			logger->parent.decorated_static,
			new_size,
			result);

	return result;
}

struct split_params {
	logger_t *original;
	csalt_static_store_block_fn *block;
	void *param;
};

static int use_split(csalt_static_store *store, void *param)
{
	struct split_params *params = param;
	logger_t copy = *params->original;
	copy.parent.decorated_static = store;
	return params->block((void*)&copy, params->param);
}

int csalt_store_logger_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	logger_t *logger = (logger_t*)store;
	struct split_params params = {
		logger,
		block,
		param,
	};

	return csalt_store_split(
		logger->parent.decorated_static,
		begin,
		end,
		use_split,
		&params);
}

const struct csalt_dynamic_store_interface impl = {
	{
		csalt_store_logger_read,
		csalt_store_logger_write,
		csalt_store_logger_split,
	},
	csalt_store_decorator_size,
	csalt_store_logger_resize,
};

struct csalt_store_logger csalt_store_logger_arrays(
	csalt_store *decorated,
	csalt_static_store *output,
	struct csalt_array error,
	struct csalt_array success,
	struct csalt_array partial_success
)
{
	return (logger_t) {
		{
			.vtable = &impl,
			.decorated = decorated,
		},
		output,
		error,
		success,
		partial_success,
	};
}

