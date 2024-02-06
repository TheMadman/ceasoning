#include "csalt/resource/logger.h"

#include "csalt/resource/format.h"
#include "csalt/log_message.h"

typedef struct csalt_resource_logger logger_t;

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_logger_init,
	csalt_resource_logger_deinit,
};

struct csalt_resource_logger csalt_resource_logger_arrays(
	csalt_resource *resource,
	csalt_static_store *output,
	struct csalt_array error,
	struct csalt_array success,
	struct csalt_array partial_success
)
{
	return (logger_t) {
		&impl,
		resource,

		csalt_store_logger_arrays(
			NULL,
			output,
			error,
			success,
			partial_success
		),
	};
}

static int use_format(csalt_store *store, void *param)
{
	csalt_static_store *output = *(csalt_static_store **)param;
	struct csalt_progress progress = csalt_progress(csalt_store_size(store));
	while (!csalt_progress_complete(&progress))
		if (csalt_store_transfer(&progress, (void*)store, output) == -1)
			return -1;
	return 0;
}

static const char *message_for(logger_t *logger, csalt_store *result)
{
	if (result)
		return csalt_log_message_get(
			logger->result.success,
			(void(*)(void))csalt_resource_init);
	else
		return csalt_log_message_get(
			logger->result.error,
			(void(*)(void))csalt_resource_init);
}

csalt_store *csalt_resource_logger_init(csalt_resource *resource)
{
	logger_t *const logger = (logger_t *)resource;
	csalt_store *result = csalt_resource_init(logger->resource);
	const char *const message = message_for(logger, result);
	if (message)
		csalt_use_format(
			use_format,
			&logger->result.output,
			"%s: csalt_resource_init(%p) -> %p\n",
			message,
			logger->resource,
			result);
	if (result) {
		logger->result.parent.decorated = result;
		return (csalt_store *)&logger->result;
	} else {
		return NULL;
	}
}

void csalt_resource_logger_deinit(csalt_resource *resource)
{
	logger_t *logger = (logger_t *)resource;
	csalt_resource_deinit(logger->resource);
}

