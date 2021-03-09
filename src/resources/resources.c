#include "csalt/resources.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "csalt/util.h"

// Memory resource functions

void csalt_memory_init(csalt_resource *resource)
{
	struct csalt_heap *memory = castto(memory, resource);
	if (!csalt_resource_valid(resource))
		memory->resource_pointer = malloc(memory->size);
}

char csalt_memory_valid(const csalt_resource *resource)
{
	struct csalt_heap *memory = (struct csalt_heap *)resource;
	return !!memory->resource_pointer;
}

void csalt_memory_deinit(csalt_resource *resource)
{
	struct csalt_heap *memory = (struct csalt_heap *)resource;
	free(memory->resource_pointer);
	memory->resource_pointer = 0;
}

// overload - attempts to initialize before writing
// do we just allow the program to segfault if init
// failed?
//
// Otherwise API users are just going to have to do the same
// init->valid->error stepss as usual, with an extra
// construct step at the beginning
ssize_t csalt_heap_write(csalt_store *resource, const void *buffer, size_t size)
{
	struct csalt_heap *memory = castto(memory, resource);
	csalt_resource_init(castto(csalt_resource *, resource));
	return csalt_memory_write(resource, buffer, size);
}

struct csalt_resource_interface memory_interface = {
	{
		csalt_memory_read,
		csalt_heap_write,
		csalt_memory_size,
		csalt_memory_split,
	},
	csalt_memory_init,
	csalt_memory_valid,
	csalt_memory_deinit,
};

struct csalt_heap csalt_heap_lazy(size_t size)
{
	struct csalt_heap result = {
		&memory_interface,
		size,
		0,
	};
	return result;
}

struct csalt_heap csalt_heap(size_t size)
{
	struct csalt_heap result = csalt_heap_lazy(size);
	csalt_resource_init(castto(csalt_resource *, &result));
	return result;
}

// Interface implementing noops and returning invalid

void csalt_noop_init(csalt_resource *_)
{
	// prevents unused parameter warnings - deliberate
	(void)_;
}

char csalt_noop_valid(const csalt_resource *_)
{
	return 0;
}

void csalt_noop_deinit(csalt_resource *_)
{
	(void)_;
}

struct csalt_resource_interface csalt_null_heap_implementation = {
	{
		csalt_store_null_read,
		csalt_store_null_write,
		csalt_store_null_size,
		csalt_store_null_split,
	},
	csalt_noop_init,
	csalt_noop_valid,
	csalt_noop_deinit,
};

const struct csalt_heap csalt_null_heap = {
	&csalt_null_heap_implementation,
	0
};

// Virtual function calls

void csalt_resource_init(csalt_resource *resource)
{
	(*resource)->init(resource);
}

char csalt_resource_valid(const csalt_resource *resource)
{
	return (*resource)->valid(resource);
}

void csalt_resource_deinit(csalt_resource *resource)
{
	(*resource)->deinit(resource);
}

struct csalt_heap csalt_resource_use(csalt_resource *resource, csalt_resource_block *code_block)
{
	csalt_resource_init(resource);
	if (!csalt_resource_valid(resource))
		return csalt_null_heap;
	struct csalt_heap result = code_block(resource);
	csalt_resource_deinit(resource);
	return result;
}


