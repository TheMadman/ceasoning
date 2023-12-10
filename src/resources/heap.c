#include "csalt/resources/heap.h"

#include <stdlib.h>
#include <string.h>

#include "csalt/util.h"

typedef struct csalt_resource_heap heap_t;
typedef struct csalt_store_heap heap_store_t;

struct csalt_resource_interface heap_impl = {
	csalt_resource_heap_init,
	csalt_resource_heap_deinit,
};

struct csalt_dynamic_store_interface heap_store_impl;

heap_t csalt_resource_heap(ssize_t size)
{
	return (heap_t) {
		&heap_impl,
		size,
		{
			.vtable = &heap_store_impl,
		}
	};
}

static heap_store_t heap_store(void *begin, void *end)
{
	return (heap_store_t) {
		&heap_store_impl,
		begin,
		end,
		0,
	};
}

csalt_store *csalt_resource_heap_init(csalt_resource *resource)
{
	heap_t *heap = (void*)resource;
	char *buffer = malloc((size_t)heap->size);
	if (!buffer)
		return NULL;

	heap->store = heap_store(buffer, buffer + heap->size);
	return (csalt_store *)&heap->store;
}

void csalt_resource_heap_deinit(csalt_resource *resource)
{
	heap_t *heap = (void*)resource;
	free(heap->store.begin);
	heap->store.begin = 0;
	heap->store.end = 0;
	heap->store.written = 0;
}

ssize_t csalt_store_heap_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
)
{
	heap_store_t *heap = (void*)store;
	size = csalt_min(size, heap->written);
	memcpy(buffer, heap->begin, (size_t)size);
	return size;
}

ssize_t csalt_store_heap_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
)
{
	heap_store_t *heap = (void*)store;
	size = csalt_min(size, heap->end - heap->begin);
	memcpy(heap->begin, buffer, (size_t)size);
	heap->written = size;
	return size;
}

int csalt_store_heap_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	heap_store_t *heap = (void*)store;
	heap_store_t tmp = heap_store(
		csalt_min(heap->begin + begin, heap->end),
		csalt_min(heap->begin + end, heap->end));

	return block((csalt_static_store*)&tmp, param);
}

ssize_t csalt_store_heap_size(csalt_store *store)
{
	heap_store_t *heap = (void*)store;
	return heap->end - heap->begin;
}

ssize_t csalt_store_heap_resize(csalt_store *store, ssize_t new_size)
{
	heap_store_t *heap = (void*)store;
	char *attempt = realloc(heap->begin, (size_t)new_size);
	if (!attempt)
		return heap->end - heap->begin;

	heap->begin = attempt;
	heap->end = heap->begin + new_size;
	return new_size;
}

struct csalt_dynamic_store_interface heap_store_impl = {
	{
		csalt_store_heap_read,
		csalt_store_heap_write,
		csalt_store_heap_split,
	},
	csalt_store_heap_size,
	csalt_store_heap_resize,
};

