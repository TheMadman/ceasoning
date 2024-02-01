#include "csalt/store/decorator.h"

typedef struct csalt_store_decorator decorator_t;

ssize_t csalt_store_decorator_read(csalt_static_store *store, void *buffer, ssize_t size)
{
	decorator_t *decorator = (void*)store;
	return csalt_store_read(decorator->decorated_static, buffer, size);
}

ssize_t csalt_store_decorator_write(csalt_static_store *store, const void *buffer, ssize_t size)
{
	decorator_t *decorator = (void*)store;
	return csalt_store_write(decorator->decorated_static, buffer, size);
}

int csalt_store_decorator_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	decorator_t *decorator = (void*)store;
	return csalt_store_split(decorator->decorated_static, begin, end, block, param);
}

ssize_t csalt_store_decorator_size(csalt_store *store)
{
	decorator_t *decorator = (void*)store;
	return csalt_store_size(decorator->decorated);
}

ssize_t csalt_store_decorator_resize(csalt_store *store, ssize_t size)
{
	decorator_t *decorator = (void*)store;
	return csalt_store_resize(decorator->decorated, size);
}

