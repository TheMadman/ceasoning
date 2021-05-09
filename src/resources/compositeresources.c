#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

#include "csalt/impl/list_impl.h"

typedef struct csalt_resource_list list;
typedef struct csalt_resource_list_initialized list_initialized;

static struct csalt_resource_interface csalt_resource_list_implementation = {
	csalt_resource_list_init,
};

static struct csalt_resource_initialized_interface csalt_resource_list_init_implementation = {
	{
		csalt_store_list_read,
		csalt_store_list_write,
		csalt_store_list_size,
		csalt_store_list_split,
	},
	csalt_resource_list_deinit,
};

struct csalt_resource_list csalt_resource_list_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	csalt_resource_initialized **buffer_begin,
	csalt_resource_initialized **buffer_end
)
{
	struct csalt_resource_list result = { 0 };
	result.vtable = &csalt_resource_list_implementation;
	result.begin = begin;
	result.end = end;
	result.list.vtable = &csalt_resource_list_init_implementation;
	result.list.parent.begin = (csalt_store **)buffer_begin;
	result.list.parent.end = (csalt_store **)buffer_end;
	return result;
}

int csalt_resource_list_init_real(
	csalt_resource **current,
	csalt_resource **end,
	csalt_resource_initialized **buffer_current,
	csalt_resource_initialized **buffer_end
)
{
	if (current == end)
		return 0;
	*buffer_current = csalt_resource_init(*current);
	if (!*buffer_current)
		return -1;
	if (
		csalt_resource_list_init_real(
			current + 1, 
			end, 
			buffer_current + 1, 
			buffer_end
		)
	) {
		csalt_resource_deinit(*buffer_current);
		return -1;
	}
	return 0;
}

size_t csalt_resource_list_length(struct csalt_resource_list_initialized *list)
{
	return list->parent.end - list->parent.begin;
}

csalt_resource_initialized *csalt_resource_list_init(csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	csalt_resource **current = list->begin;
	csalt_resource **end = list->end;
	csalt_resource_initialized **buffer_current = castto(buffer_current, list->list.parent.begin);
	csalt_resource_initialized **buffer_end = castto(buffer_end, list->list.parent.end);
	if (csalt_resource_list_init_real(current, end, buffer_current, buffer_end))
		return 0;
	return (csalt_resource_initialized *)&list->list;
}

void csalt_resource_list_deinit(csalt_resource_initialized *resource)
{
	struct csalt_resource_list_initialized *list = castto(list, resource);
	for (
		csalt_resource_initialized
			**current = castto(current, list->parent.begin),
			**end = castto(current, list->parent.end);
		current < end;
		current++
	) {
		csalt_resource_deinit(*current);
	}
}

typedef struct csalt_store_list store_list;

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

static struct csalt_resource_initialized_interface csalt_resource_fallback_init_implementation = {
	{
		csalt_store_fallback_read,
		csalt_store_fallback_write,
		csalt_store_fallback_size,
		csalt_store_fallback_split,
	},
	csalt_resource_list_deinit,
};

static struct csalt_resource_interface csalt_resource_fallback_implementation = {
	csalt_resource_fallback_init,
};

struct csalt_resource_fallback csalt_resource_fallback_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_fallback result = { 0 };
	result.begin = begin;
	result.end = end;
	result.vtable = &csalt_resource_fallback_implementation;

	result.fallback.vtable = &csalt_resource_fallback_init_implementation;

	return result;
}

csalt_resource_initialized *csalt_resource_fallback_init(csalt_resource *resource)
{
	struct csalt_resource_fallback *fallback = castto(fallback, resource);
	csalt_resource **current = fallback->begin;
	csalt_resource **end = fallback->end;
	csalt_resource_initialized **buffer_current = castto(buffer_current, fallback->fallback.parent.list.begin);
	csalt_resource_initialized **buffer_end = castto(buffer_end, fallback->fallback.parent.list.end);

	if (csalt_resource_list_init_real(current, end, buffer_current, buffer_end))
		return 0;
	return (csalt_resource_initialized *)&fallback->fallback;
}

// the csalt_resource_first doesn't actually need to split every resource,
// so the regular resource interface will do

static struct csalt_resource_interface csalt_resource_first_implementation = {
	csalt_resource_first_init,
};

static struct csalt_resource_initialized_interface csalt_resource_first_init_implementation = {
	{
		csalt_resource_first_read,
		csalt_resource_first_write,
		csalt_resource_first_size,
		csalt_resource_first_split,
	},
	csalt_resource_first_deinit,
};

struct csalt_resource_first csalt_resource_first_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_first result = { 0 };
	result.vtable = &csalt_resource_first_implementation;
	result.begin = begin;
	result.end = end;

	result.first.initialized = 0;
	result.first.vtable = &csalt_resource_first_init_implementation;
	return result;
}

csalt_resource_initialized *csalt_resource_first_init(csalt_resource *resource)
{
	struct csalt_resource_first *first = castto(first, resource);

	for (
		csalt_resource
			**current = castto(current, first->begin),
			**end = castto(end, first->end);
		current < end;
		current++
	) {
		csalt_resource_initialized *result = csalt_resource_init(*current);
		if (result) {
			first->first.initialized = result;
			return &first->first.vtable;
		}
	}

	return 0;
}

void csalt_resource_first_deinit(csalt_resource_initialized *resource)
{
	struct csalt_resource_first_initialized *first = (void*)resource;
	csalt_resource_deinit(first->initialized);
	first->initialized = 0;
}

ssize_t csalt_resource_first_read(const csalt_store *store, void *buffer, size_t size)
{
	struct csalt_resource_first_initialized *first = (void*)store;
	return csalt_store_read(csalt_store(first->initialized), buffer, size);
}

ssize_t csalt_resource_first_write(csalt_store *store, const void *buffer, size_t size)
{
	struct csalt_resource_first_initialized *first = (void*)store;
	return csalt_store_write(csalt_store(first->initialized), buffer, size);
}

size_t csalt_resource_first_size(const csalt_store *store)
{
	struct csalt_resource_first_initialized *first = (void*)store;
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
	struct csalt_resource_first_initialized *first = (void*)store;
	return csalt_store_split(csalt_store(first->initialized), begin, end, block, data);
}

