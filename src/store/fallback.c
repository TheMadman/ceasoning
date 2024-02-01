#include "csalt/store/fallback.h"

typedef struct csalt_store_fallback fallback;
typedef struct csalt_static_store_fallback static_fallback;

const struct csalt_dynamic_store_interface csalt_fallback_implementation = {
	{
		csalt_store_fallback_read,
		csalt_store_fallback_write,
		csalt_store_fallback_split,
	},
	csalt_store_fallback_size,
	csalt_store_fallback_resize,
};

fallback csalt_store_fallback(
	csalt_store *primary,
	csalt_store *secondary
)
{
	return (fallback) {
		&csalt_fallback_implementation,
		csalt_store_pair(primary, secondary),
	};
}

static_fallback csalt_static_store_fallback(
	csalt_static_store *primary,
	csalt_static_store *secondary
)
{
	return (static_fallback) {
		&csalt_fallback_implementation.parent,
		csalt_static_store_pair(primary, secondary),
	};
}

struct read_params {
	void *buffer;
	ssize_t remaining;
};

static int receive_split(
	csalt_static_store *store,
	void *param
)
{
	struct csalt_static_store_pair *pair
		= (struct csalt_static_store_pair *)store;

	struct read_params *params = param;
	ssize_t *remaining = &params->remaining;

	const ssize_t transferred = csalt_store_read(
		pair->second,
		params->buffer,
		*remaining);

	if (transferred < 0)
		return -1;

	csalt_store_write(
		pair->first,
		params->buffer,
		transferred);

	*remaining -= transferred;
	return 0;
}

ssize_t csalt_store_fallback_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	static_fallback *const fb = (static_fallback*)store;
	const ssize_t first_read = csalt_store_read(
		fb->pair.first,
		buffer,
		amount);
	if (first_read == amount)
		return first_read;
	if (first_read < 0)
		return first_read;

	struct read_params params = {
		((char*)buffer) + first_read,
		amount - first_read,
	};

	const int second_read_code = csalt_store_split(
		(csalt_static_store*)&fb->pair,
		first_read,
		amount,
		receive_split,
		&params);

	if (second_read_code < 0)
		return second_read_code;

	return amount - params.remaining;
}

ssize_t csalt_store_fallback_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	static_fallback *const fb = (static_fallback*)store;
	return csalt_store_write((csalt_static_store*)fb->pair.first, buffer, amount);
}

struct split_params {
	csalt_static_store_block_fn *block;
	void *param;
};

static int receive_pair(csalt_static_store *store, void *param)
{
	struct csalt_static_store_pair *pair
		= (struct csalt_static_store_pair*)store;
	struct split_params *params = param;

	static_fallback fb = csalt_static_store_fallback(pair->first, pair->second);

	return params->block((csalt_static_store*)&fb, params->param);
}

int csalt_store_fallback_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	static_fallback *const fb = (static_fallback*)store;
	struct split_params params = {block, param};
	return csalt_store_split(
		(csalt_static_store*)&fb->pair,
		begin,
		end,
		receive_pair,
		&params);
}

ssize_t csalt_store_fallback_size(csalt_store *store)
{
	fallback *const fb = (fallback*)store;
	return csalt_store_size((csalt_store*)&fb->pair);
}

ssize_t csalt_store_fallback_resize(
	csalt_store *store,
	ssize_t new_size
)
{
	fallback *const fb = (fallback*)store;
	return csalt_store_resize((csalt_store*)&fb->pair, new_size);
}

ssize_t csalt_store_fallback_flush(
	static_fallback *store,
	ssize_t amount
)
{
	struct csalt_progress progress = csalt_progress(amount);
	return csalt_store_transfer(
		&progress,
		store->pair.first,
		store->pair.second);
}

int csalt_store_fallback_array_bounds(
	csalt_store **begin,
	csalt_store **end,
	struct csalt_store_fallback *out_begin,
	struct csalt_store_fallback *out_end
)
{
	if (begin > end)
		return -1;

	if (out_begin > out_end)
		return -1;

	if (end - begin > out_end - out_begin)
		return -1;

	for (; begin < end - 1; begin++, out_begin++) {
		*out_begin = csalt_store_fallback(
			*begin,
			(csalt_store *)(out_begin + 1)
		);
	}

	*out_begin = csalt_store_fallback(*begin, NULL);
	return 0;
}

