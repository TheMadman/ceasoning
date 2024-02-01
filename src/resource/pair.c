#include "csalt/resource/pair.h"

typedef struct csalt_resource_pair pair_t;

static const struct csalt_dynamic_resource_interface impl = {
	csalt_resource_pair_init,
	csalt_resource_pair_deinit,
};

struct csalt_resource_pair csalt_resource_pair(
	csalt_resource *first,
	csalt_resource *second
)
{
	return (pair_t) {
		&impl,
		first,
		second,
		{ 0 },
	};
}

void csalt_resource_pair_list_bounds(
	csalt_resource **begin,
	csalt_resource **end,
	struct csalt_resource_pair *pairs_begin,
	struct csalt_resource_pair *pairs_end
)
{
	if (begin >= end)
		return;

	if (pairs_begin >= pairs_end)
		return;

	if (pairs_end - pairs_begin < end - begin)
		return;

	for (; begin < end - 1; begin++, pairs_begin++) {
		*pairs_begin = csalt_resource_pair(
			*begin,
			(csalt_resource *)(pairs_begin + 1)
		);
	}
	*pairs_begin = csalt_resource_pair(*begin, 0);
}

static csalt_store *const init_if(csalt_resource *resource)
{
	if (resource)
		return csalt_resource_init(resource);
	return NULL;
}

csalt_store *csalt_resource_pair_init(csalt_resource *resource)
{
	pair_t *pair = (pair_t *)resource;

	csalt_store *const first = init_if(pair->first);
	if (pair->first && !first)
		return NULL;

	csalt_store *const second = init_if(pair->second);
	if (pair->second && !second) {
		if (first)
			csalt_resource_deinit(pair->first);
		return NULL;
	}

	pair->result = csalt_store_pair(first, second);
	return (csalt_store *)&pair->result;
}

void csalt_resource_pair_deinit(csalt_resource *resource)
{
	pair_t *pair = (pair_t *)resource;

	if (pair->second)
		csalt_resource_deinit(pair->second);
	if (pair->first)
		csalt_resource_deinit(pair->first);
}
