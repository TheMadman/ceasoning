#include "csalt/resources.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "csalt/util.h"

// Memory resource functions
static struct csalt_resource_interface heap_implementation;
static struct csalt_resource_initialized_interface heap_initialized_implementation;

ssize_t csalt_heap_write(csalt_store *store, const void *buffer, size_t size)
{
	ssize_t written = csalt_memory_write(store, buffer, size);
	if (written < 0)
		return -1;

	struct csalt_heap_initialized *heap = castto(heap, store);
	heap->amount_written = max(heap->amount_written, written);
	return written;
}

ssize_t csalt_heap_read(const csalt_store *store, void *buffer, size_t size)
{
	struct csalt_heap_initialized *heap = castto(heap, store);
	size = min(size, heap->amount_written);

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
	struct csalt_heap_initialized *heap = castto(heap, store);
	if (csalt_store_size(store) < end)
		return -1;

	// so common, should it be in the virtual call function?
	// Can't think of a case where a heap_split accepts an end before
	// a beginning
	if (end <= begin)
		return -1;

	struct csalt_heap_initialized split = *heap;
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

csalt_resource_initialized *csalt_heap_init(csalt_resource *resource)
{
	struct csalt_heap *memory = castto(memory, resource);
	char *result = malloc(memory->heap.size);
	if (result) {
		memory->heap.parent.begin = result;
		memory->heap.parent.end = result + memory->heap.size;
		return (csalt_resource_initialized *)&memory->heap;
	}
	return 0;
}

void csalt_heap_deinit(csalt_resource_initialized *resource)
{
	struct csalt_heap_initialized *memory = castto(memory, resource);
	free(memory->parent.begin);
	memory->parent.begin = 0;
	memory->parent.end = 0;
}

static struct csalt_resource_interface heap_implementation = {
	csalt_heap_init,
};

static struct csalt_resource_initialized_interface initialized_heap_implementation = {
	{
		csalt_heap_read,
		csalt_heap_write,
		csalt_memory_size,
		csalt_heap_split,
	},
	csalt_heap_deinit,
};

struct csalt_heap csalt_heap(size_t size)
{
	struct csalt_heap result = { 0 };
	result.vtable = &heap_implementation;
	result.heap.vtable = &initialized_heap_implementation;
	result.heap.size = size;
	return result;
}

void *csalt_resource_heap_raw(const struct csalt_heap_initialized *heap)
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

// Virtual function calls

csalt_resource_initialized *csalt_resource_init(csalt_resource *resource)
{
	return (*resource)->init(resource);
}

void csalt_resource_deinit(csalt_resource_initialized *resource)
{
	(*resource)->deinit(resource);
}

int csalt_resource_use(
	csalt_resource *resource,
	csalt_store_block_fn *code_block,
	void *data
)
{
	csalt_resource_initialized *initialized = csalt_resource_init(resource);
	if (!initialized)
		return -1;
	int result = code_block((csalt_store *)initialized, data);
	csalt_resource_deinit(initialized);
	return result;
}


