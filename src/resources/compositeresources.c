#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

#include "csalt/impl/list_impl.h"

typedef struct csalt_resource_list list;
typedef struct csalt_resource_list_initialized list_initialized;

static struct csalt_resource_interface csalt_resource_list_implementation = {
	csalt_resource_list_init,
	csalt_resource_list_deinit,
};

struct csalt_resource_list csalt_resource_list_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	csalt_store **buffer_begin,
	csalt_store **buffer_end
)
{
	struct csalt_resource_list result = { 0 };
	result.vtable = &csalt_resource_list_implementation;
	result.begin = begin;
	result.end = end;
	result.list = csalt_store_list_bounds(buffer_begin, buffer_end);
	return result;
}

int csalt_resource_list_init_real(
	csalt_resource **current,
	csalt_resource **end,
	csalt_store **buffer_current,
	csalt_store **buffer_end
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
		csalt_resource_deinit(*current);
		return -1;
	}
	return 0;
}

csalt_store *csalt_resource_list_init(csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	csalt_resource **current = list->begin;
	csalt_resource **end = list->end;
	csalt_store **buffer_current = list->list.begin;
	csalt_store **buffer_end = list->list.end;
	if (csalt_resource_list_init_real(current, end, buffer_current, buffer_end))
		return 0;
	return (csalt_store *)&list->list;
}

void csalt_resource_list_deinit(csalt_resource *resource)
{
	struct csalt_resource_list *list = (struct csalt_resource_list *)resource;
	for (
		csalt_resource
			**current = list->begin,
			**end = list->end;
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

static struct csalt_resource_interface csalt_resource_fallback_implementation = {
	csalt_resource_fallback_init,
	csalt_resource_fallback_deinit,
};

struct csalt_resource_fallback csalt_resource_fallback_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	csalt_store **buffer_begin,
	csalt_store **buffer_end
)
{
	struct csalt_resource_fallback result = { 0 };
	result.begin = begin;
	result.end = end;
	result.vtable = &csalt_resource_fallback_implementation;

	result.fallback = csalt_store_fallback_bounds(buffer_begin, buffer_end);

	return result;
}

csalt_store *csalt_resource_fallback_init(csalt_resource *resource)
{
	struct csalt_resource_fallback *fallback = (void *)resource;
	csalt_resource **current = fallback->begin;
	csalt_resource **end = fallback->end;
	csalt_store **buffer_current = castto(buffer_current, fallback->fallback.list.begin);
	csalt_store **buffer_end = castto(buffer_end, fallback->fallback.list.end);

	if (csalt_resource_list_init_real(current, end, buffer_current, buffer_end))
		return 0;
	return (csalt_store *)&fallback->fallback;
}

void csalt_resource_fallback_deinit(csalt_resource *resource)
{
	struct csalt_resource_fallback *fallback = (void *)resource;
	for (
		csalt_resource
			**current = fallback->begin,
			**end = fallback->end;
		current < end;
		current++
	) {
		csalt_resource_deinit(*current);
	}
}

static struct csalt_resource_interface csalt_resource_first_implementation = {
	csalt_resource_first_init,
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

	return result;
}

csalt_store *csalt_resource_first_init(csalt_resource *resource)
{
	struct csalt_resource_first *first = (void *)resource;

	for (
		csalt_resource
			**current = castto(current, first->begin),
			**end = castto(end, first->end);
		current < end;
		current++
	) {
		csalt_store *result = csalt_resource_init(*current);
		if (result) {
			first->returned = *current;
			return result;
		}
	}

	return 0;
}

void csalt_resource_first_deinit(csalt_resource *resource)
{
	struct csalt_resource_first *first = (void *)resource;
	if (first->returned) {
		csalt_resource_deinit(first->returned);
		first->returned = 0;
	}
}

