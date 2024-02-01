#include "csalt/store/mutex.h"

typedef struct csalt_store_mutex mutex_t;

static const struct csalt_dynamic_store_interface impl = {
	{
		csalt_store_mutex_read,
		csalt_store_mutex_write,
		csalt_store_mutex_split,
	},
	csalt_store_decorator_size,
	csalt_store_decorator_resize,
};

struct csalt_store_mutex csalt_store_mutex(
	csalt_store *decorated,
	csalt_mutex *mutex
)
{
	return (mutex_t) {
		{
			.vtable = &impl,
			.decorated = decorated,
		},
		mutex,
	};
}

ssize_t csalt_store_mutex_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	mutex_t *mutex = (mutex_t*)store;

	if (csalt_mutex_trylock(mutex->mutex) != 0)
		return -1;

	const ssize_t result = csalt_store_read(
		mutex->parent.decorated_static,
		buffer,
		amount);
	csalt_mutex_unlock(mutex->mutex);
	return result;
}

ssize_t csalt_store_mutex_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	mutex_t *mutex = (mutex_t*)store;

	if (csalt_mutex_trylock(mutex->mutex) != 0)
		return -1;

	const ssize_t result = csalt_store_write(
		mutex->parent.decorated_static,
		buffer,
		amount);
	csalt_mutex_unlock(mutex->mutex);
	return result;
}

int csalt_store_mutex_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	mutex_t *mutex = (mutex_t*)store;

	if (csalt_mutex_trylock(mutex->mutex) != 0)
		return -1;

	const int result = csalt_store_split(
		mutex->parent.decorated_static,
		begin,
		end,
		block,
		param);

	csalt_mutex_unlock(mutex->mutex);

	return result;
}

