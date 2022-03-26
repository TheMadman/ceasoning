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

#include <csalt/compositeresources.h>
#include <csalt/compositestores.h>

static struct csalt_resource_interface csalt_resource_pair_implementation = {
	csalt_resource_pair_init,
	csalt_resource_pair_deinit,
};

struct csalt_resource_pair csalt_resource_pair(
	csalt_resource *first,
	csalt_resource *second
)
{
	struct csalt_resource_pair result = {
		&csalt_resource_pair_implementation,
		first,
		second,
		{ 0 }
	};

	return result;
}

int csalt_resource_pair_list_bounds(
	csalt_resource **in_begin,
	csalt_resource **in_end,
	struct csalt_resource_pair *out_begin,
	struct csalt_resource_pair *out_end
)
{
	if (in_begin > in_end)
		return -1;

	if (out_begin > out_end)
		return -1;

	// if (arrlength(resources) > arrlength(out))
	if (in_end - in_begin > out_end - out_begin)
		return -1;

	for(; in_begin < in_end - 1; in_begin++, out_begin++)
		*out_begin = csalt_resource_pair(
			*in_begin,
			(void *)(out_begin + 1)
		);

	*out_begin = csalt_resource_pair(*in_begin, 0);

	return 0;
}

csalt_store *csalt_resource_pair_init(csalt_resource *resource)
{
	struct csalt_resource_pair *pair = (void *)resource;
	csalt_store *first = 0;
	if (pair->first)
		if (!(first = csalt_resource_init(pair->first)))
			return 0;

	csalt_store *second = 0;
	if (pair->second)
		if (!(second = csalt_resource_init(pair->second))) {
			if (first)
				csalt_resource_deinit(pair->first);
			return 0;
		}

	pair->result = csalt_store_pair(first, second);
	return csalt_store(&pair->result);
}

void csalt_resource_pair_deinit(csalt_resource *resource)
{
	struct csalt_resource_pair *pair = (void *)resource;
	if (pair->result.first) {
		csalt_resource_deinit(pair->first);
		pair->result.first = 0;
	}
	if (pair->result.second) {
		csalt_resource_deinit(pair->second);
		pair->result.second = 0;
	}
}

static struct csalt_resource_interface
	csalt_resource_fallback_implementation = {
	csalt_resource_fallback_init,
	csalt_resource_fallback_deinit,
};

struct csalt_resource_fallback csalt_resource_fallback(
	csalt_resource *first,
	csalt_resource *second
)
{
	struct csalt_resource_fallback result = {
		&csalt_resource_fallback_implementation,
		csalt_resource_pair(first, second),
		{ 0 }
	};

	return result;
}

int csalt_resource_fallback_bounds(
	csalt_resource **in_begin,
	csalt_resource **in_end,
	struct csalt_resource_fallback *out_begin,
	struct csalt_resource_fallback *out_end
)
{
	if (in_end <= in_begin)
		return -1;

	if (out_end <= out_begin)
		return -1;

	// if (arrlength(in) > arrlength(out))
	if (in_end - in_begin > out_end - out_begin)
		return -1;

	for (; in_begin < in_end - 1; in_begin++, out_begin++) {
		*out_begin = csalt_resource_fallback(
			*in_begin,
			(void *)(out_begin + 1)
		);
	}

	*out_begin = csalt_resource_fallback(*in_begin, 0);

	return 0;
}

static struct csalt_resource_interface csalt_resource_first_implementation = {
	csalt_resource_first_init,
	csalt_resource_first_deinit,
};

struct csalt_resource_first csalt_resource_first_bounds(
	csalt_resource **begin,
	csalt_resource **end
)
{
	struct csalt_resource_first result = { 0 };
	result.vtable = &csalt_resource_first_implementation;
	result.begin = begin;
	result.end = end;

	return result;
}

csalt_store *csalt_resource_fallback_init(csalt_resource *resource)
{
	struct csalt_resource_fallback *fallback = (void *)resource;

	struct csalt_store_pair
		*pair = (void *)csalt_resource_init(
			csalt_resource(&fallback->pair)
		);

	fallback->result = csalt_store_fallback(pair->first, pair->second);
	return (void *)&fallback->result;
}

void csalt_resource_fallback_deinit(csalt_resource *resource)
{
	struct csalt_resource_fallback *fallback = (void *)resource;

	csalt_resource_deinit(csalt_resource(&fallback->pair));
}

csalt_store *csalt_resource_first_init(csalt_resource *resource)
{
	struct csalt_resource_first *first = (void *)resource;

	for (
		csalt_resource
			**current = (csalt_resource **)first->begin,
			**end = (csalt_resource **)first->end;
		current < end;
		current++
	) {
		csalt_store *result = csalt_resource_init(*current);
		if (result) {
			first->returned = *current;
			return result;
		}
	}

	return 0;
}

void csalt_resource_first_deinit(csalt_resource *resource)
{
	struct csalt_resource_first *first = (void *)resource;
	if (first->returned) {
		csalt_resource_deinit(first->returned);
		first->returned = 0;
	}
}

