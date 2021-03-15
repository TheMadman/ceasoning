#include "csalt/resources.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "csalt/util.h"

// Memory resource functions

void csalt_heap_init(csalt_resource *resource)
{
	struct csalt_heap *memory = castto(memory, resource);
	if (!csalt_resource_valid(resource)) {
		char *result = malloc(memory->size);
		memory->parent.begin = result;
		memory->parent.end = result + memory->size;
	}
}

char csalt_heap_valid(const csalt_resource *resource)
{
	struct csalt_heap *memory = (struct csalt_heap *)resource;
	return !!memory->parent.begin;
}

void csalt_heap_deinit(csalt_resource *resource)
{
	struct csalt_heap *memory = (struct csalt_heap *)resource;
	free(memory->parent.begin);
	memory->parent.begin = 0;
	memory->parent.end = 0;
}

ssize_t csalt_heap_write(csalt_store *store, const void *buffer, size_t size)
{
	ssize_t written = csalt_memory_write(store, buffer, size);
	if (written < 0)
		return -1;

	struct csalt_heap *heap = castto(heap, store);
	heap->amount_written = max(heap->amount_written, written);
	return written;
}

ssize_t csalt_heap_read(const csalt_store *store, void *buffer, size_t size)
{
	struct csalt_heap *heap = castto(heap, store);
	if (size < heap->amount_written)
		return -1;

	return csalt_memory_read(store, buffer, size);
}

int csalt_heap_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_heap *heap = castto(heap, store);
	if (csalt_store_size(store) < end)
		return -1;

	// so common, should it be in the virtual call function?
	// Can't think of a case where a heap_split accepts an end before
	// a beginning
	if (end <= begin)
		return -1;

	struct csalt_heap split = *heap;
	split.parent.begin = heap->parent.begin + begin;
	split.parent.end = heap->parent.begin + end;

	ssize_t written_overlap_amount = max(0, min(end, heap->amount_written) - begin);
	split.amount_written = written_overlap_amount;

	int result = block(csalt_store(&split), data);

	// the whole point of this function - if block appends past current written,
	// update amount_written
	heap->amount_written = max(heap->amount_written, begin + split.amount_written);
	return result;
}

struct csalt_resource_interface heap_interface = {
	{
		csalt_heap_read,
		csalt_heap_write,
		csalt_memory_size,
		csalt_heap_split,
	},
	csalt_heap_init,
	csalt_heap_valid,
	csalt_heap_deinit,
};

struct csalt_heap csalt_heap_lazy(size_t size)
{
	struct csalt_heap result = {
		{
			&heap_interface.parent,
			0,
		},
		size,
		0,
	};
	return result;
}

struct csalt_heap csalt_heap(size_t size)
{
	struct csalt_heap result = csalt_heap_lazy(size);
	csalt_resource_init(csalt_resource(&result));
	return result;
}

void *csalt_resource_heap_raw(const struct csalt_heap *heap)
{
	return csalt_store_memory_raw(&heap->parent);
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
	{
		&csalt_null_heap_implementation.parent,
		0,
	},
	0,
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


