#include "csalt/resource/mutex.h"

typedef struct csalt_resource_mutex mutex_t;

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_mutex_init,
	csalt_resource_mutex_deinit,
};

struct csalt_resource_mutex csalt_resource_mutex(
	csalt_resource *resource,
	csalt_mutex_params *params
)
{
	return (mutex_t) {
		.vtable = &impl,
		.resource = resource,
		.params = params,
	};
}

csalt_store *csalt_resource_mutex_init(csalt_resource *resource)
{
	mutex_t *mutex = (mutex_t *)resource;
	if (csalt_mutex_init(&mutex->mutex, mutex->params))
		return NULL;

	csalt_store *const decorated = csalt_resource_init(mutex->resource);
	if (!decorated) {
		csalt_mutex_deinit(&mutex->mutex);
		return NULL;
	}

	mutex->result = csalt_store_mutex(decorated, &mutex->mutex);
	return (csalt_store *)&mutex->result;
}

void csalt_resource_mutex_deinit(csalt_resource *resource)
{
	mutex_t *mutex = (mutex_t *)resource;
	csalt_resource_deinit(mutex->resource);
	csalt_mutex_deinit(&mutex->mutex);
}
