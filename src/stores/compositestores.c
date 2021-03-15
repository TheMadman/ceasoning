#include <csalt/compositestores.h>

#include <csalt/baseresources.h>
#include <csalt/util.h>

#include <stdint.h>

struct csalt_store_interface csalt_store_fallback_interface = {
	csalt_store_fallback_read,
	csalt_store_fallback_write,
	csalt_store_fallback_size,
	csalt_store_fallback_split,
};

csalt_store *csalt_store_list_get(
	const struct csalt_store_list *store,
	size_t index
)
{
	if (store->begin + index >= store->end) {
		return 0;
	}

	return castto(csalt_store *, store->begin[index]);
}

size_t csalt_store_list_length(const struct csalt_store_list *store)
{
	return store->end - store->begin;
}

struct csalt_store_fallback csalt_store_fallback_bounds(
	csalt_store **begin,
	csalt_store **end
)
{
	struct csalt_store_fallback result = {
		&csalt_store_fallback_interface,
		begin,
		end,
	};

	return result;
}

ssize_t csalt_store_fallback_read(
	const csalt_store *store,
	void *buffer,
	size_t size
)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	ssize_t result = -1;

	if (fallback->list.begin == fallback->list.end)
		return result;

	result = csalt_store_read(*fallback->list.begin, buffer, size);
	if (result < size) {
		struct csalt_store_fallback subcalls = *fallback;
		subcalls.list.begin++;
		result = csalt_store_fallback_read(csalt_store(&subcalls), buffer, size);
		if (result == size) {
			struct csalt_transfer progress = csalt_transfer(size);

			// Don't really error-check - if this write fails,
			// the whole fallback should gracefully... fall... back
			// Acts as a blocking transfer deliberately
			for (size_t transferred = 0; transferred < size;) {
				csalt_store_transfer(
					&progress,
					*fallback->list.begin,
					csalt_store(&subcalls),
					0
				);
			}
		}
	}

	return result;
}

ssize_t csalt_store_fallback_write(
	csalt_store *store,
	const void *buffer,
	size_t size
)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	return csalt_store_write(*fallback->list.begin, buffer, size);
}

struct split_data {
	csalt_store **current_source;
	csalt_store **current_destination;
	struct csalt_store_fallback *result;
	struct resource_heap_data *heap_data;
};

struct resource_heap_data {
	struct csalt_heap heap;
	struct csalt_store_fallback *fallback;
	size_t begin;
	size_t end;
	csalt_store_block_fn *block;
	void *data_param;
	int error;
};

static void manage_splitting(struct split_data *split_data);

static int receive_single_split_store(csalt_store *store, void *data)
{
	struct split_data *split_data = data;

	*split_data->current_destination = store;
	split_data->current_destination++;
	split_data->current_source++;

	manage_splitting(split_data);
	return 0;
}

static void manage_splitting(struct split_data *split_data)
{
	csalt_store **current = split_data->current_source;
	csalt_store **list_end = split_data->heap_data->fallback->list.end;
	csalt_store_block_fn *block = split_data->heap_data->block;
	void *data_param = split_data->heap_data->data_param;
	int *error_out = &split_data->heap_data->error;

	if (current >= list_end) {
		*error_out = block(castto(csalt_store *, split_data->result), data_param);
		return;
	}

	split_data->heap_data->error = csalt_store_split(
		*split_data->current_source,
		split_data->heap_data->begin,
		split_data->heap_data->end,
		receive_single_split_store,
		split_data
	);
}

static struct csalt_heap use_heap_memory(csalt_resource *resource)
{
	struct resource_heap_data *data = castto(data, resource);
	
	csalt_store **begin = data->fallback->list.begin;
	csalt_store **end = data->fallback->list.end;

	size_t length = end - begin;
	csalt_store **raw_pointer = csalt_store_memory_raw(
		castto(struct csalt_memory *, &data->heap)
	);

	struct csalt_store_fallback result = csalt_store_fallback_bounds(
		raw_pointer,
		raw_pointer + length
	);

	struct split_data split_data = {
		begin,
		result.list.begin,
		&result,
		data,
	};

	manage_splitting(&split_data);

	return csalt_null_heap;
}

/*
 * High-level view of this algorithm:
 * - Allocate heap memory for split sub-store pointers
 * - For each store, split it, save the pointer in the heap
 * - Set that heap as the list for another fallback store
 * - Finally, pass that store and *data to block()
 */
int csalt_store_fallback_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	struct resource_heap_data heap_data = {
		csalt_heap_lazy(
			sizeof(csalt_store *) *
			csalt_store_list_length(&fallback->list)
		),
		fallback,
		begin,
		end,
		block,
		data,
		-1
	};
	csalt_resource_use(
		castto(csalt_resource *, &heap_data),
		use_heap_memory
	);
	return heap_data.error;
}

size_t csalt_store_fallback_size(const csalt_store *store)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	size_t result = UINTMAX_MAX;
	for(
		csalt_store **current = fallback->list.begin;
		current < fallback->list.end;
		current++
	) {
		result = min(result, csalt_store_size(*current));
	}

	return result;
}



