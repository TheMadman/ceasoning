#include <csalt/compositestores.h>

#include <csalt/baseresources.h>
#include <csalt/util.h>

#include <stdint.h>
#include <limits.h>

struct csalt_store_interface csalt_store_list_implementation = {
	csalt_store_list_read,
	csalt_store_list_write,
	csalt_store_list_size,
	csalt_store_list_split,
};

typedef csalt_store_list_receive_split_fn receive_split_fn;

struct csalt_store_list csalt_store_list_bounds(
	csalt_store **begin,
	csalt_store **end
)
{
	struct csalt_store_list result = {
		&csalt_store_list_implementation,
		begin,
		end,
	};
	return result;
}

csalt_store *csalt_store_list_get(
	const struct csalt_store_list *store,
	size_t index
)
{
	if (store->begin + index >= store->end) {
		return 0;
	}

	return store->begin[index];
}

ssize_t csalt_store_list_read(const csalt_store *store, void *buffer, size_t amount)
{
	struct csalt_store_list *list = castto(list, store);
	ssize_t result = -1;

	for (csalt_store **current = list->begin; current < list->end; current++) {
		result = max(result, csalt_store_read(*current, buffer, amount));
		if (result < 0)
			continue;

		if (result == amount)
			return result;
	}

	return result;
}

ssize_t csalt_store_list_write(csalt_store *store, const void *buffer, size_t amount)
{
	struct csalt_store_list *list = castto(list, store);
	ssize_t result = SSIZE_MAX;

	for (csalt_store **current = list->begin; current < list->end; current++) {
		result = min(result, csalt_store_write(*current, buffer, amount));
	}

	return result;
}

size_t csalt_store_list_size(const csalt_store *store)
{
	struct csalt_store_list *list = castto(list, store);
	size_t result = SIZE_MAX;
	for(
		csalt_store **current = list->begin;
		current < list->end;
		current++
	) {
		result = min(result, csalt_store_size(*current));
	}

	return result;
}

int csalt_store_list_receive_split(
	struct csalt_store_list *original,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	(void)original;
	(void)begin;
	(void)end;
	return block(csalt_store(list), data);
}

struct csalt_store_list_split_params {
	struct csalt_store_list *original;
	size_t begin;
	size_t end;
	csalt_store_block_fn *block;
	void *data;

	int error;
};

struct store_single_split_params {
	csalt_store
		**in_current,
		**in_end,
		**out_begin,
		**out_current;
	struct csalt_store_list_split_params *list_params;
};

int manage_splitting(struct store_single_split_params *params);
int receive_single_split(csalt_store *store, void *param)
{
	struct store_single_split_params *params = param;

	*params->out_current = store;
	
	params->in_current++;
	params->out_current++;

	return manage_splitting(params);
}

int manage_splitting(struct store_single_split_params *params)
{
	csalt_store
		**current = params->in_current,
		**end = params->in_end;

	size_t
		split_begin = params->list_params->begin,
		split_end = params->list_params->end;

	if (current == end) {
		struct csalt_store_list list = csalt_store_list_bounds(
			params->out_begin,
			params->out_current
		);
		params->list_params->error = params->list_params->block(
			(csalt_store *)&list,
			params->list_params->data
		);
		return 0;
	}

	return csalt_store_split(
		*current,
		split_begin,
		split_end,
		receive_single_split,
		params
	);
}

int store_list_split_use_heap(csalt_store *resource, void *params)
{
	struct csalt_store_list_split_params *list_params = params;
	struct csalt_heap_initialized *heap = (struct csalt_heap_initialized *)resource;

	csalt_store
		**begin = list_params->original->begin,
		**end = list_params->original->end,
		**out_begin = csalt_resource_heap_raw(heap),
		**out_current = out_begin;

	struct store_single_split_params single_params = {
		begin,
		end,
		out_begin,
		out_current,
		list_params,
	};

	return manage_splitting(&single_params);
}

int csalt_store_list_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_store_list *list = (struct csalt_store_list *)store;
	size_t list_length = csalt_store_list_length(list);
	struct csalt_heap heap = csalt_heap(sizeof(csalt_store *) * list_length);

	struct csalt_store_list_split_params params = {
		list,
		begin,
		end,
		block,
		data,

		-1,
	};

	csalt_resource_use((csalt_resource *)&heap, store_list_split_use_heap, &params);

	return params.error;
}

size_t csalt_store_list_length(const struct csalt_store_list *store)
{
	return store->end - store->begin;
}

struct csalt_store_interface csalt_store_fallback_implementation = {
	csalt_store_fallback_read,
	csalt_store_fallback_write,
	csalt_store_fallback_size,
	csalt_store_fallback_split,
};

struct csalt_store_fallback csalt_store_fallback_bounds(
	csalt_store **begin,
	csalt_store **end
)
{
	struct csalt_store_fallback result = {
		{
			&csalt_store_fallback_implementation,
			begin,
			end,
		},
		0,
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
	if (result < (ssize_t)size) {
		struct csalt_store_fallback subcalls = *fallback;
		subcalls.list.begin++;
		result = csalt_store_fallback_read(csalt_store(&subcalls), buffer, size);
		if (result == size) {
			struct csalt_transfer progress = csalt_transfer(size);

			// Don't really error-check - if this write fails,
			// the whole fallback should gracefully... fall... back
			// Acts as a blocking transfer deliberately
			for (size_t transferred = 0; transferred < size;) {
				transferred = csalt_store_transfer(
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
	fallback->amount_written = max(
		fallback->amount_written,
		csalt_store_write(*fallback->list.begin, buffer, size)
	);
	return fallback->amount_written;
}

size_t csalt_store_fallback_size(const csalt_store *store)
{
	struct csalt_store_fallback *fallback = castto(fallback, store);
	size_t result = SIZE_MAX;
	for(
		csalt_store **current = fallback->list.begin;
		current < fallback->list.end;
		current++
	) {
		result = min(result, csalt_store_size(*current));
	}

	return result;
}

struct fallback_split_list_params {
	struct csalt_store_fallback *original;
	size_t begin;
	size_t end;
	csalt_store_block_fn *block;
	void *data;
};

static int fallback_receive_split_list(
	csalt_store *new_store,
	void *data
)
{
	struct fallback_split_list_params *params = data;
	struct csalt_store_list *new_list = (struct csalt_store_list *)new_store;
	struct csalt_store_fallback *old_fallback = params->original;
	struct csalt_store_fallback new_fallback = csalt_store_fallback_bounds(
		new_list->begin,
		new_list->end
	);
	ssize_t written_end = min(old_fallback->amount_written, params->end);
	ssize_t amount_written_deducted = written_end - params->begin;
	ssize_t new_amount_written = max(0, amount_written_deducted);
	new_fallback.amount_written = new_amount_written;
	int result = params->block((csalt_store *)&new_fallback, params->data);

	old_fallback->amount_written = max(
		old_fallback->amount_written,
		params->begin + new_fallback.amount_written
	);
	return result;
}

int csalt_store_fallback_split(
	csalt_store *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct fallback_split_list_params params = {
		(struct csalt_store_fallback *)list,
		begin,
		end,
		block,
		data
	};

	return csalt_store_list_split(
		list,
		begin,
		end,
		fallback_receive_split_list,
		&params
	);
}

struct csalt_transfer csalt_store_fallback_flush(
	struct csalt_store_fallback *fallback,
	struct csalt_transfer *transfers
)
{
	struct csalt_transfer result = { 0 };
	if (!fallback->amount_written)
		return result;

	csalt_store **first = fallback->list.begin;
	csalt_store **current = first + 1;
	result.total = fallback->amount_written * (fallback->list.end - first);
	for (; current < fallback->list.end; current++, transfers++) {
		transfers->total = fallback->amount_written;
		transfers->amount_completed = csalt_store_transfer(
			transfers,
			*current,
			*first,
			0
		);
		result.amount_completed += transfers->amount_completed;
	}

	return result;
}

