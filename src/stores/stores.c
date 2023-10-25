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

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "csalt/stores/base.h"
#include "csalt/util.h"

// virtual call functions

ssize_t csalt_static_store_read(
	csalt_static_store *from,
	void *to,
	ssize_t bytes
)
{
	return (*from)->read(from, to, bytes);
}

ssize_t csalt_static_store_write(
	csalt_static_store *to,
	const void *from,
	ssize_t bytes
)
{
	return (*to)->write(to, from, bytes);
}

int csalt_static_store_split(
	csalt_static_store *store,
	ssize_t start,
	ssize_t end,
	csalt_store_fn *block,
	void *data
)
{
	return (*store)->split(store, start, end, block, data);
}

ssize_t csalt_dynamic_store_size(csalt_store *store)
{
	return (*store)->size(store);
}

ssize_t csalt_dynamic_store_resize(
	csalt_store *store,
	ssize_t new_size
)
{
	return (*store)->resize(store, new_size);
}

// Transfer algorithm

struct csalt_progress csalt_progress(ssize_t size)
{
	struct csalt_progress result = {
		size,
		0
	};
	return result;
}

ssize_t csalt_progress_remaining(const struct csalt_progress *progress)
{
	return progress->total - progress->amount_completed;
}

int csalt_progress_complete(const struct csalt_progress *progress)
{
	return progress->total == progress->amount_completed;
}

static int transfer_split(csalt_static_store *store, void *params)
{
	char buffer[DEFAULT_PAGESIZE] = { 0 };

	struct csalt_progress *progress = params;
	struct csalt_static_store_pair *pair = (void *)store;

	ssize_t amount = csalt_min(
		(ssize_t)sizeof(buffer),
		csalt_progress_remaining(progress)
	);

	ssize_t amount_read = csalt_static_store_read(
		pair->first,
		buffer,
		amount
	);

	if (amount_read < 0) {
		return -1;
	}

	ssize_t amount_write = csalt_static_store_write(
		pair->second,
		buffer,
		csalt_min(amount, amount_read)
	);

	if (amount_write < 0) {
		return -1;
	}

	progress->amount_completed += amount_write;

	return csalt_progress_complete(progress);
}

ssize_t csalt_store_transfer(
	struct csalt_progress *progress,
	csalt_static_store *from,
	csalt_static_store *to
)
{
	if (csalt_progress_complete(progress)) {
		return 0;
	}

	const struct csalt_static_store_pair pair = csalt_static_store_pair(
		from,
		to
	);

	int attempt = csalt_static_store_split(
		(csalt_static_store *)&pair,
		progress->amount_completed,
		progress->total,
		transfer_split,
		progress
	);

	if (attempt < 0)
		return attempt;
	return progress->amount_completed;
}

