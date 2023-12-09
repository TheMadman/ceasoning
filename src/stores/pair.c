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

#include "csalt/stores/pair.h"

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "csalt/stores/base.h"
#include "csalt/util.h"

static const struct csalt_dynamic_store_interface csalt_store_pair_implementation = {
	{
		csalt_store_pair_read,
		csalt_store_pair_write,
		csalt_store_pair_split,
	},
	csalt_store_pair_size,
	csalt_store_pair_resize,
};

struct csalt_store_pair csalt_store_pair(csalt_store *first, csalt_store *second)
{
	struct csalt_store_pair result = {
		&csalt_store_pair_implementation,
		first,
		second,
	};
	return result;
}

struct csalt_static_store_pair csalt_static_store_pair(
	csalt_static_store *first,
	csalt_static_store *second
)
{
	return (struct csalt_static_store_pair) {
		&csalt_store_pair_implementation.parent,
		first,
		second,
	};
}

void csalt_store_pair_list_bounds(
	csalt_store **begin,
	csalt_store **end,
	struct csalt_store_pair *out_begin,
	struct csalt_store_pair *out_end
)
{
	if (begin >= end)
		return;

	if (out_begin >= out_end)
		return;

	// arrlength(stores) > arrlength(out)
	if (end - begin > out_end - out_begin)
		return;

	for (; begin < end - 1; begin++, out_begin++) {
		*out_begin = csalt_store_pair(*begin, (csalt_store *)(out_begin + 1));
	}
	*out_begin = csalt_store_pair(*begin, 0);
}

ssize_t csalt_store_pair_read(csalt_static_store *store, void *buffer, ssize_t size)
{
	const struct csalt_static_store_pair *pair = (void *)store;
	ssize_t first = 0;
	if (pair->first)
		first = csalt_store_read(pair->first, buffer, size);

	if (first < 0)
		return first;

	ssize_t second = first;
	if (first == 0 && pair->second)
		second = csalt_store_read(pair->second, buffer, size);

	if (second < 0)
		return second;

	return csalt_max(first, second);
}

ssize_t csalt_store_pair_write(csalt_static_store *store, const void *buffer, ssize_t size)
{
	struct csalt_static_store_pair *pair = (void *)store;
	if (!(pair->first || pair->second))
		return 0;

	ssize_t first = SSIZE_MAX;
	if (pair->first)
		first = csalt_store_write(pair->first, buffer, size);

	if (first < 0)
		return first;

	ssize_t second = first;
	if (pair->second)
		second = csalt_store_write(pair->second, buffer, size);

	return csalt_min(first, second);
}

ssize_t csalt_store_pair_size(csalt_store *store)
{
	struct csalt_store_pair *pair = (void *)store;
	ssize_t first = 0;
	if (pair->first)
		first = csalt_store_size(pair->first);

	ssize_t second = first;
	if (pair->second)
		second = csalt_store_size(pair->second);

	return csalt_min(first, second);
}

ssize_t csalt_store_pair_resize(csalt_store *store, ssize_t new_size)
{
	struct csalt_store_pair *pair = (void *)store;
	if (!(pair->first || pair->second))
		return -1;

	ssize_t first = new_size;

	if (pair->first)
		first = csalt_store_resize(pair->first, new_size);

	ssize_t second = new_size;
	if (pair->second)
		second = csalt_store_resize(pair->second, new_size);

	return csalt_min(first, second);
}

struct split_pair_params {
	csalt_static_store *store;
	ssize_t begin;
	ssize_t end;
	csalt_static_store_block_fn *block;
	void *param;

	csalt_static_store *pair_first;
};

static int split_receive_second(csalt_static_store *store, void *param)
{
	struct split_pair_params *params = param;
	struct csalt_static_store_pair new_pair
		= csalt_static_store_pair(params->pair_first, store);

	return params->block((void *)&new_pair, params->param);
}

static int split_receive_first(csalt_static_store *store, void *param)
{
	struct split_pair_params *params = param;
	struct csalt_static_store_pair *original = (void *)(params->store);
	params->pair_first = store;

	if (!original->second) {
		struct csalt_static_store_pair new_pair
			= csalt_static_store_pair(store, 0);
		return params->block((void *)&new_pair, params->param);
	}

	return csalt_store_split(
		original->second,
		params->begin,
		params->end,
		split_receive_second,
		params
	);
}

int csalt_store_pair_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	struct split_pair_params params = {
		store,
		begin,
		end,
		block,
		param,

		0,
	};
	struct csalt_static_store_pair *pair = (void *)store;

	if (!(pair->first || pair->second)) {
		return block(store, param);
	}

	if (pair->first)
		return csalt_store_split(
			pair->first,
			begin,
			end,
			split_receive_first,
			&params
		);
	else
		return csalt_store_split(
			pair->second,
			begin,
			end,
			split_receive_second,
			&params
		);
}

ssize_t csalt_store_pair_list_length(const struct csalt_store_pair *pairs)
{
	ssize_t size = 0;
	for (; pairs; pairs = (struct csalt_store_pair *)pairs->second)
		size++;
	return size;
}

csalt_store *csalt_store_pair_list_get(
	const struct csalt_store_pair *pairs,
	ssize_t index
)
{
	if (!pairs)
		return 0;
	if (index == 0)
		return pairs->first;
	return csalt_store_pair_list_get((void *)pairs->second, index - 1);
}

struct csalt_store_multisplit_params {
	struct csalt_static_store_pair *list;
	const struct csalt_store_multisplit_split *begin;
	const struct csalt_store_multisplit_split *end;
	csalt_static_store_block_fn *block;
	void *param;

	csalt_static_store *first;
};

static int multisplit_receive_second(csalt_static_store *store, void *param)
{
	struct csalt_store_multisplit_params *params = param;
	struct csalt_static_store_pair pair
		= csalt_static_store_pair(params->first, store);
	return params->block((csalt_static_store *)&pair, params->param);
}

static int multisplit_receive_first(csalt_static_store *store, void *param)
{
	struct csalt_store_multisplit_params *params = param;
	params->first = store;
	return csalt_store_pair_list_multisplit_bounds(
		(struct csalt_static_store_pair *)params->list->second,
		params->begin + 1,
		params->end,
		multisplit_receive_second,
		params
	);
}

int csalt_store_pair_list_multisplit_bounds(
	struct csalt_static_store_pair *list,
	const struct csalt_store_multisplit_split *begin,
	const struct csalt_store_multisplit_split *end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	const bool finished_list = !list;
	const bool finished_split_array = begin == end;
	if (
		finished_list ||
		finished_split_array
	)
		return block((csalt_static_store *)list, param);

	const bool first = list->first;
	const bool second = list->second;

	if (!(first || second)) {
		return block((csalt_static_store *)list, param);
	}

	struct csalt_store_multisplit_params params = {
		list,
		begin,
		end,
		block,
		param,

		NULL,
	};

	// (!first && !second) handled above
	csalt_static_store *child_store = first? list->first: list->second;
	csalt_static_store_block_fn *path = first?
		multisplit_receive_first:
		multisplit_receive_second;

	return csalt_store_split(
		child_store,
		begin->begin,
		begin->end,
		path,
		&params
	);
}

