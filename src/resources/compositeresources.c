#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

struct csalt_resource_interface csalt_resource_list_implementation = {
	{
		csalt_store_list_read,
		csalt_store_list_write,
		csalt_store_list_size,
		csalt_store_list_split,
	},
	csalt_resource_list_init,
	csalt_resource_list_valid,
	csalt_resource_list_deinit,
};

struct csalt_resource_list csalt_resource_list_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_list result = {
		csalt_store_list_bounds((csalt_store **)begin, (csalt_store **)end)
	};
	result.parent.vtable = castto(
		result.parent.vtable,
		&csalt_resource_list_implementation
	);
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

void csalt_resource_list_init(csalt_resource *resource)
{
	struct csalt_resource_list *list = castto(list, resource);
	csalt_resource **current = castto(current, list->parent.begin);
	csalt_resource **end = castto(end, list->parent.end);
	csalt_resource_list_init_real(current, end);
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

