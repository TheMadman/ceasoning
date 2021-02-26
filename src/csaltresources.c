#include "csaltresources.h"

#include <stdlib.h>

// Virtual function calls

void csalt_resource_init(csalt_resource *resource)
{
	return (*resource)->init(resource);
}

void *csalt_resource_pointer(csalt_resource *resource)
{
	return (*resource)->get_pointer(resource);
}

char csalt_resource_valid(csalt_resource *resource)
{
	return (*resource)->valid(csalt_resource_pointer(resource));
}

void csalt_resource_deinit(csalt_resource *resource)
{
	return (*resource)->deinit(csalt_resource_pointer(resource));
}

void *csalt_use(csalt_resource *resource, csalt_resource_block *code_block)
{
	csalt_resource_init(resource);
	if (!csalt_resource_valid(resource))
		return NULL;
	void *result = code_block(csalt_resource_pointer(resource));
	csalt_resource_deinit(resource);
	return result;
}

// Memory resource functions

void csalt_memory_init(csalt_resource *resource)
{
	csalt_memory *memory = (csalt_memory *)resource;
	if (!csalt_resource_valid(resource))
		memory->resource_pointer = malloc(memory->size);
}

void *csalt_memory_pointer(csalt_resource *resource)
{
	csalt_memory *memory = (csalt_memory *)resource;
	return memory->resource_pointer;
}

char csalt_memory_valid(void *pointer)
{
	return !!pointer;
}

void csalt_memory_deinit(void *pointer)
{
	free(pointer);
}

const struct csalt_resource_interface memory_interface = {
	csalt_memory_init,
	csalt_memory_pointer,
	csalt_memory_valid,
	csalt_memory_deinit,
};

csalt_memory csalt_memory_make(size_t size)
{
	csalt_memory result = {
		&memory_interface,
		size,
		0,
	};
	return result;
}

