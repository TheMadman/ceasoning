#include "csalt/decoratorresources.h"

csalt_store *csalt_resource_decorator_init(csalt_resource *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_init(decorator->child);
}

void csalt_resource_decorator_deinit(csalt_resource *resource)
{
	struct csalt_resource_decorator *decorator = (void *)resource;
	return csalt_resource_deinit(decorator->child);
}
