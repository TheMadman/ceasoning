#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

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

