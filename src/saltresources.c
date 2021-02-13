#include "saltresources.h"

#include <stdlib.h>

// Virtual function calls

void *salt_resource_pointer(salt_resource *resource)
{
	return (*resource)->get_pointer(resource);
}

char salt_resource_valid(salt_resource *resource)
{
	return (*resource)->valid(salt_resource_pointer(resource));
}

void salt_resource_deinit(salt_resource *resource)
{
	return (*resource)->deinit(salt_resource_pointer(resource));
}

void *salt_use(salt_resource *resource, salt_resource_block *code_block)
{
	if (!salt_resource_valid(resource))
		return NULL;
	void *result = code_block(salt_resource_pointer(resource));
	salt_resource_deinit(resource);
	return result;
}

// Memory resource functions

void *salt_memory_pointer(salt_resource *resource)
{
	salt_memory *memory = (salt_memory *)resource;
	return memory->resource_pointer;
}

char salt_memory_valid(void *pointer)
{
	return !!pointer;
}

void salt_memory_deinit(void *pointer)
{
	free(pointer);
}

const struct salt_resource_interface memory_interface = {
	salt_memory_pointer,
	salt_memory_valid,
	salt_memory_deinit,
};

salt_memory salt_memory_init(size_t size)
{
	salt_memory result = {
		&memory_interface,
		malloc(size),
	};
	return result;
}

