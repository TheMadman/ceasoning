#include "csalt/resource/first.h"

typedef struct csalt_resource_first first_t;

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_first_init,
	csalt_resource_first_deinit,
};

struct csalt_resource_first csalt_resource_first(
	csalt_resource *first,
	csalt_resource *second
)
{
	return (first_t) {
		{
			&impl,
			first,
			second,
		},
	};
}

void csalt_resource_first_list_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	struct csalt_resource_first *first_begin,
	struct csalt_resource_first *first_end
)
{
	if (begin >= end)
		return;

	if (first_begin >= first_end)
		return;

	if (end - begin > first_end - first_begin)
		return;

	for (; begin < end - 1; begin++, first_begin++)
		*first_begin = csalt_resource_first(
			*begin,
			(csalt_resource *)(first_begin + 1));

	*first_begin = csalt_resource_first(*begin, 0);
}

static csalt_store *const init_if(csalt_resource *resource)
{
	if (resource)
		return csalt_resource_init(resource);
	return NULL;
}

csalt_store *csalt_resource_first_init(csalt_resource *resource)
{
	first_t *first = (first_t *)resource;

	csalt_store *const first_store = init_if(first->parent.first);
	if (first_store) {
		first->initialized = first->parent.first;
		return first_store;
	} else {
		csalt_store *const second_store = init_if(first->parent.second);
		if (second_store)
			first->initialized = first->parent.second;
		return second_store;
	}
}

void csalt_resource_first_deinit(csalt_resource *resource)
{
	first_t *first = (first_t *)resource;
	csalt_resource_deinit(first->initialized);
}

