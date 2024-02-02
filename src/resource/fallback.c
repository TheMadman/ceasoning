#include "csalt/resource/fallback.h"

typedef struct csalt_resource_fallback fallback_t;
typedef struct csalt_store_fallback fallback_store_t;

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_fallback_init,
	csalt_resource_fallback_deinit,
};

struct csalt_resource_fallback csalt_resource_fallback(
	csalt_resource *first,
	csalt_resource *second
)
{
	return (struct csalt_resource_fallback) {
		&impl,
		first,
		second,
		{ 0 },
	};
}

void csalt_resource_fallback_list_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	struct csalt_resource_fallback *fallback_begin,
	struct csalt_resource_fallback *fallback_end
)
{
	if (begin >= end)
		return;

	if (fallback_begin >= fallback_end)
		return;

	if (end - begin > fallback_end - fallback_begin)
		return;

	for (; begin < end - 1; begin++, fallback_begin++)
		*fallback_begin = csalt_resource_fallback(
			*begin,
			(csalt_resource *)(fallback_begin + 1));

	*fallback_begin = csalt_resource_fallback(
		*begin,
		NULL);
}

static csalt_store *const init_if(csalt_resource *resource)
{
	if (resource)
		return csalt_resource_init(resource);
	return NULL;
}

csalt_store *csalt_resource_fallback_init(csalt_resource *resource)
{
	fallback_t *fallback = (fallback_t *)resource;
	csalt_store *const first = init_if(fallback->first);
	if (fallback->first && !first)
		return NULL;

	csalt_store *const second = init_if(fallback->second);
	if (fallback->second && !second) {
		if (first)
			csalt_resource_deinit(fallback->first);
		return NULL;
	}

	fallback->result = csalt_store_fallback(first, second);
	return (csalt_store *)&fallback->result;
}

void csalt_resource_fallback_deinit(csalt_resource *resource)
{
	fallback_t *fallback = (fallback_t *)resource;
	if (fallback->first)
		csalt_resource_deinit(fallback->first);
	if (fallback->second)
		csalt_resource_deinit(fallback->second);
}

