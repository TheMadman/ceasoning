#include "csalt/resource/lazy.h"

typedef struct csalt_resource_lazy lazy_t;
typedef struct csalt_store_lazy lazy_store_t;

static const struct csalt_dynamic_store_interface store_impl = {
	{
		csalt_store_lazy_read,
		csalt_store_lazy_write,
		csalt_store_lazy_split,
	},
	csalt_store_lazy_size,
	csalt_store_lazy_resize,
};

static lazy_store_t csalt_store_lazy(csalt_resource *resource)
{
	return (lazy_store_t) {
		&store_impl,
		resource,
		NULL,
	};
}

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_lazy_init,
	csalt_resource_lazy_deinit,
};

struct csalt_resource_lazy csalt_resource_lazy(csalt_resource *resource)
{
	return (lazy_t) {
		&impl,
		csalt_store_lazy(resource),
	};
}

csalt_store *csalt_resource_lazy_init(csalt_resource *resource)
{
	lazy_t *lazy = (lazy_t *)resource;
	return (csalt_store *)&lazy->result;
}

void csalt_resource_lazy_deinit(csalt_resource *resource)
{
	lazy_t *lazy = (lazy_t *)resource;
	if (lazy->result.store) {
		csalt_resource_deinit(lazy->result.resource);
		lazy->result.store = NULL;
	}
}

static csalt_store *init_if(lazy_store_t *lazy)
{
	if (!lazy->store)
		lazy->store = csalt_resource_init(lazy->resource);
	return lazy->store;
}

ssize_t csalt_store_lazy_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	lazy_store_t *lazy = (lazy_store_t *)store;
	csalt_static_store *initialized = (csalt_static_store *)init_if(lazy);
	if (initialized)
		return csalt_store_read(initialized, buffer, amount);
	return -1;
}

ssize_t csalt_store_lazy_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	lazy_store_t *lazy = (lazy_store_t *)store;
	csalt_static_store *initialized = (csalt_static_store *)init_if(lazy);
	if (initialized)
		return csalt_store_write(initialized, buffer, amount);
	return -1;
}

int csalt_store_lazy_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	lazy_store_t *lazy = (lazy_store_t *)store;
	csalt_static_store *initialized = (csalt_static_store *)init_if(lazy);
	if (initialized)
		return csalt_store_split(
			initialized,
			begin,
			end,
			block,
			param);
	return -1;
}

ssize_t csalt_store_lazy_size(csalt_store *store)
{
	lazy_store_t *lazy = (lazy_store_t *)store;
	csalt_store *initialized = init_if(lazy);
	if (initialized)
		return csalt_store_size(initialized);
	return -1;
}

ssize_t csalt_store_lazy_resize(csalt_store *store, ssize_t new_size)
{
	lazy_store_t *lazy = (lazy_store_t *)store;
	csalt_store *initialized = init_if(lazy);
	if (initialized)
		return csalt_store_resize(initialized, new_size);
	return -1;
}

