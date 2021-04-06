#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

#include "csalt/impl/list_impl.h"

static struct csalt_resource_list_interface csalt_resource_list_init_implementation = {
	{
		{
			csalt_store_list_read,
			csalt_store_list_write,
			csalt_store_list_size,
			csalt_resource_list_split,
		},
		csalt_noop_init,
		csalt_resource_list_valid,
		csalt_resource_list_deinit,
	},
	csalt_resource_list_receive_split,
};

static struct csalt_resource_list_interface csalt_resource_list_implementation = {
	{
		{
			csalt_store_null_read,
			csalt_store_null_write,
			csalt_store_null_size,
			csalt_store_null_split,
		},
		csalt_resource_list_init,
		csalt_noop_valid,
		csalt_noop_deinit,
	},
};

struct csalt_resource_list csalt_resource_list_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_list result = {
		.parent = csalt_store_list_bounds((csalt_store **)begin, (csalt_store **)end)
	};
	result.vtable = &csalt_resource_list_implementation;
	return result;
}

int csalt_resource_list_init_real(csalt_resource **current, csalt_resource **end)
{
	if (current == end)
		return 0;
	csalt_resource_init(*current);
	if (!csalt_resource_valid(*current))
		return -1;
	if (csalt_resource_list_init_real(current + 1, end)) {
		csalt_resource_deinit(*current);
		return -1;
	}
	return 0;
}

csalt_resource *csalt_resource_list_get(
	struct csalt_resource_list *list,
	size_t index
)
{
	return csalt_resource(csalt_store_list_get(&list->parent, index));
}

size_t csalt_resource_list_length(struct csalt_resource_list *list)
{
	return list->parent.end - list->parent.begin;
}

void csalt_resource_list_init(csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	csalt_resource **current = castto(current, list->parent.begin);
	csalt_resource **end = castto(end, list->parent.end);
	if (!csalt_resource_list_init_real(current, end))
		list->vtable = &csalt_resource_list_init_implementation;
}

char csalt_resource_list_valid(const csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	for (
		csalt_resource
			**current = castto(current, list->parent.begin),
			**end = castto(end, list->parent.end);
		current < end;
		current++
	) {
		if (!csalt_resource_valid(*current))
			return 0;
	}
	return 1;
}

void csalt_resource_list_deinit(csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	for (
		csalt_resource
			**current = castto(current, list->parent.begin),
			**end = castto(current, list->parent.end);
		current < end;
		current++
	) {
		csalt_resource_deinit(*current);
	}
}

int csalt_resource_list_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_resource_list *list = castto(list, store);
	struct csalt_resource_list_interface *vtable = castto(vtable, list->parent.vtable);
	struct resource_heap_data heap_data = {
		csalt_heap_lazy(
			sizeof(csalt_store *) *
			csalt_resource_list_length(list)
		),
		csalt_store_list(list),
		begin,
		end,
		block,
		data,
		-1,
		vtable->receive_split_list,
	};
	return manage_heap_data(&heap_data);
}

int csalt_resource_list_receive_split(
	struct csalt_store_list *original,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	(void)original;
	(void)begin;
	(void)end;
	return block(csalt_store(list), data);
}

static struct csalt_resource_list_interface csalt_resource_fallback_init_implementation = {
	{
		{
			csalt_store_fallback_read,
			csalt_store_fallback_write,
			csalt_store_fallback_size,
			csalt_resource_list_split,
		},
		csalt_noop_init,
		csalt_resource_list_valid,
		csalt_resource_list_deinit,
	},
	csalt_resource_list_receive_split,
};

static struct csalt_resource_list_interface csalt_resource_fallback_implementation = {
	{
		{
			csalt_store_null_read,
			csalt_store_null_write,
			csalt_store_null_size,
			csalt_store_null_split,
		},
		csalt_resource_fallback_init,
		csalt_noop_valid,
		csalt_noop_deinit,
	},
	0,
};

struct csalt_resource_fallback csalt_resource_fallback_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_fallback result = {
		.parent = csalt_store_fallback_bounds((csalt_store **)begin, (csalt_store **)end),
	};

	result.vtable = &csalt_resource_fallback_implementation;

	return result;
}

void csalt_resource_fallback_init(csalt_resource *resource)
{
	csalt_resource_list_init(resource);

	struct csalt_resource_fallback *fallback = castto(fallback, resource);
	if (fallback->vtable == &csalt_resource_list_init_implementation)
		fallback->vtable = &csalt_resource_fallback_init_implementation;
}

// the csalt_resource_first doesn't actually need to split every resource,
// so the regular resource interface will do

static struct csalt_resource_interface csalt_resource_first_implementation = {
	{
		csalt_store_null_read,
		csalt_store_null_write,
		csalt_store_null_size,
		csalt_store_null_split,
	},
	csalt_resource_first_init,
	csalt_noop_valid,
	csalt_noop_deinit,
};

static struct csalt_resource_interface csalt_resource_first_init_implementation = {
	{
		csalt_resource_first_read,
		csalt_resource_first_write,
		csalt_resource_first_size,
		csalt_resource_first_split,
	},
	csalt_noop_init,
	csalt_resource_first_valid,
	csalt_resource_first_deinit,
};

struct csalt_resource_first csalt_resource_first_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_first result = {
		.parent = csalt_resource_list_bounds(begin, end),
		.initialized = 0,
	};

	result.vtable = &csalt_resource_first_implementation;
	return result;
}

void csalt_resource_first_init(csalt_resource *resource)
{
	struct csalt_resource_first *first = castto(first, resource);
	struct csalt_resource_list *list = &first->parent;

	for (
		csalt_resource
			**current = castto(current, list->parent.begin),
			**end = castto(end, list->parent.end);
		current < end;
		current++
	) {
		csalt_resource_init(*current);
		if (csalt_resource_valid(*current)) {
			first->initialized = *current;
			first->vtable = &csalt_resource_first_init_implementation;
			return;
		}
	}
}

char csalt_resource_first_valid(const csalt_resource *resource)
{
	struct csalt_resource_first *first = castto(first, resource);
	return !!first->initialized;
}

void csalt_resource_first_deinit(csalt_resource *resource)
{
	struct csalt_resource_first *first = castto(first, resource);
	csalt_resource_deinit(first->initialized);
	first->initialized = 0;
	first->vtable = &csalt_resource_first_implementation;
}

ssize_t csalt_resource_first_read(const csalt_store *store, void *buffer, size_t size)
{
	struct csalt_resource_first *first = castto(first, store);
	return csalt_store_read(csalt_store(first->initialized), buffer, size);
}

ssize_t csalt_resource_first_write(csalt_store *store, const void *buffer, size_t size)
{
	struct csalt_resource_first *first = castto(first, store);
	return csalt_store_write(csalt_store(first->initialized), buffer, size);
}

size_t csalt_resource_first_size(const csalt_store *store)
{
	struct csalt_resource_first *first = castto(first, store);
	return csalt_store_size(csalt_store(first->initialized));
}

int csalt_resource_first_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_resource_first *first = castto(first, store);
	return csalt_store_split(csalt_store(first->initialized), begin, end, block, data);
}

