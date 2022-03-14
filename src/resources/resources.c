/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "csalt/resources.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "csalt/util.h"

// Memory resource functions
static struct csalt_resource_interface heap_implementation;

ssize_t csalt_heap_write(csalt_store *store, const void *buffer, ssize_t size)
{
	struct csalt_heap_initialized *heap = (void *)store;
	ssize_t written = csalt_memory_write((csalt_store *)&heap->memory, buffer, size);
	if (written < 0)
		return -1;
	heap->amount_written = max(heap->amount_written, written);
	return written;
}

ssize_t csalt_heap_read(csalt_store *store, void *buffer, ssize_t size)
{
	struct csalt_heap_initialized *heap = (void *)store;
	size = min(size, heap->amount_written);

	return csalt_memory_read((csalt_store *)&heap->memory, buffer, size);
}

int csalt_heap_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
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
	split.memory.begin = heap->memory.begin + begin;
	split.memory.end = heap->memory.begin + end;

	ssize_t written_overlap_amount = max(0, min(end, heap->amount_written) - begin);
	split.amount_written = written_overlap_amount;

	int result = block(csalt_store(&split), data);

	// the whole point of this function - if block appends past current written,
	// update amount_written
	heap->amount_written = max(heap->amount_written, begin + split.amount_written);
	return result;
}

csalt_store *csalt_heap_init(csalt_resource *resource)
{
	struct csalt_heap *memory = castto(memory, resource);
	char *result = calloc(memory->heap.size, 1);
	if (result) {
		memory->heap.memory = csalt_memory_bounds(result, result + memory->heap.size);
		return (csalt_store *)&memory->heap;
	}
	return 0;
}

void csalt_heap_deinit(csalt_resource *resource)
{
	struct csalt_heap *heap = (struct csalt_heap *)resource;
	struct csalt_heap_initialized *memory = (void *)&heap->heap;
	if (memory->memory.begin)
		free(memory->memory.begin);
	memory->memory.begin = 0;
	memory->memory.end = 0;
}

static struct csalt_resource_interface heap_implementation = {
	csalt_heap_init,
	csalt_heap_deinit,
};

static struct csalt_store_interface initialized_heap_implementation = {
	csalt_heap_read,
	csalt_heap_write,
	csalt_memory_size,
	csalt_heap_split,
};

struct csalt_heap csalt_heap(ssize_t size)
{
	struct csalt_heap result = { 0 };
	result.vtable = &heap_implementation;
	result.heap.vtable = &initialized_heap_implementation;
	result.heap.size = size;
	return result;
}

void *csalt_resource_heap_raw(const struct csalt_heap_initialized *heap)
{
	return csalt_memory_raw(&heap->memory);
}

static struct csalt_resource_vector_initialized vector_initialized(
	void *allocated,
	void *allocated_end,
	ssize_t begin,
	ssize_t end,
	ssize_t write_amount
);

ssize_t csalt_resource_vector_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
)
{
	struct csalt_resource_vector_initialized *vector = (void *)store;

	ssize_t read_amount = min(amount, vector->amount_written);
	memcpy(buffer, vector->original_pointer + vector->begin, read_amount);
	return read_amount;
}

struct vector_write_params {
	const void *buffer;
	ssize_t amount;

	ssize_t result;
};

int vector_write(csalt_store *store, void *arg)
{
	struct csalt_resource_vector_initialized *vector = (void *)store;
	struct vector_write_params *params = arg;
	void *write_pointer = vector->original_pointer + vector->begin;
	ssize_t write_amount = min(
		params->amount,
		vector->original_end - write_pointer
	);


	memcpy(
		write_pointer,
		params->buffer,
		write_amount
	);
	vector->amount_written = write_amount;
	params->result = write_amount;
	return 0;
}

ssize_t csalt_resource_vector_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
)
{
	struct csalt_resource_vector_initialized *vector = (void *)store;

	struct vector_write_params params = {
		buffer,
		amount,

		-1,
	};

	csalt_store_split(
		store,
		0,
		amount,
		vector_write,
		&params
	);

	return params.result;
}

ssize_t csalt_resource_vector_size(csalt_store *store)
{
	struct csalt_resource_vector_initialized *vector = (void *)store;
	return vector->end - vector->begin;
}

int csalt_resource_vector_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *arg
)
{
	struct csalt_resource_vector_initialized *vector = (void *)store;

	ssize_t begin_index = vector->begin + begin;
	ssize_t end_index = vector->begin + end;

	void **allocated = &vector->original_pointer;
	void **allocated_end = &vector->original_end;

	int needs_realloc = *allocated + begin_index > *allocated_end ||
		*allocated + end_index > *allocated_end;

	if (needs_realloc) {
		ssize_t size = *allocated_end - *allocated;

		// Value where the ssize_t would overflow on <<
		ssize_t overflow_boundary = (SSIZE_MAX >> 1) + 1;

		// I'm almost certain I saw research showing that
		// something like 1.76 is an "optimal" growth factor,
		// but I can't find it anymore so I'm using powers 
		// of 2 for simplicity's sake
		while (size < end_index && size < overflow_boundary) {
			size = size << 1;
		}

		void *reallocated = realloc(*allocated, size);
		if (reallocated) {
			// sets the values in *vector as well
			*allocated = reallocated;
			*allocated_end = reallocated + size;
		} else {
			begin_index = min(
				begin_index, 
				*allocated_end - *allocated
			);
			end_index = min(
				end_index,
				*allocated_end - *allocated
			);
		}
	}

	ssize_t written_from_begin = max(vector->amount_written - begin, 0);

	struct csalt_resource_vector_initialized result = vector_initialized(
		*allocated,
		*allocated_end,
		begin_index,
		end_index,
		written_from_begin
	);

	int return_value = block(csalt_store(&result), arg);

	if (result.amount_written > written_from_begin) {
		ssize_t delta = result.amount_written - written_from_begin;
		vector->amount_written += delta;
	}

	return return_value;
}

struct csalt_store_interface vector_initialized_implementation = {
	csalt_resource_vector_read,
	csalt_resource_vector_write,
	csalt_resource_vector_size,
	csalt_resource_vector_split,
};

static struct csalt_resource_vector_initialized vector_initialized(
	void *allocated,
	void *allocated_end,
	ssize_t begin,
	ssize_t end,
	ssize_t write_amount
)
{
	struct csalt_resource_vector_initialized result = {
		&vector_initialized_implementation,
		allocated,
		allocated_end,
		begin,
		end,
		write_amount,
	};
	return result;
}

csalt_store *csalt_resource_vector_init(csalt_resource *resource)
{
	struct csalt_resource_vector *vector_resource = (void *)resource;

	if (vector_resource->vector.original_pointer)
		return csalt_store(&vector_resource->vector);

	// min size chosen arbitrarily
	// I should really have better grounding for my decisions but w/e
	ssize_t alloc_size = 1 << 5;
	while (alloc_size < vector_resource->size)
		alloc_size = alloc_size << 1;

	void *result = calloc(alloc_size, 1);
	if (!result)
		return 0;

	vector_resource->vector = vector_initialized(
		result,
		result + alloc_size,
		0,
		SSIZE_MAX,
		0
	);

	return csalt_store(&vector_resource->vector);
}

void csalt_resource_vector_deinit(csalt_resource *resource)
{
	struct csalt_resource_vector *vector_resource = (void *)resource;
	if (vector_resource->vector.original_pointer) {
		free(vector_resource->vector.original_pointer);
		vector_resource->vector = vector_initialized(0,0,0,0,0);
	}
}

struct csalt_resource_interface csalt_resource_vector_implementation = {
	csalt_resource_vector_init,
	csalt_resource_vector_deinit,
};

struct csalt_resource_vector csalt_resource_vector(ssize_t size)
{
	struct csalt_resource_vector result = {
		&csalt_resource_vector_implementation,
		size,
		{ 0 }
	};
	return result;
}

// Interface implementing noops and returning invalid

void csalt_noop_init(csalt_resource *_)
{
	// prevents unused parameter warnings - deliberate
	(void)_;
}

void csalt_noop_deinit(csalt_resource *_)
{
	(void)_;
}

// Virtual function calls

csalt_store *csalt_resource_init(csalt_resource *resource)
{
	return (*resource)->init(resource);
}

void csalt_resource_deinit(csalt_resource *resource)
{
	(*resource)->deinit(resource);
}

int csalt_resource_use(
	csalt_resource *resource,
	csalt_store_block_fn *code_block,
	void *data
)
{
	csalt_store *initialized = csalt_resource_init(resource);
	if (!initialized)
		return -1;
	int result = code_block(initialized, data);
	csalt_resource_deinit(resource);
	return result;
}


