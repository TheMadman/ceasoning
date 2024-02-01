/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2023   Marcus Harrison
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

#include "csalt/store/noop.h"

bool csalt_store_error(const csalt_store * const store)
{
	return !(store && store != csalt_store_noop);
}

bool csalt_static_store_error(const csalt_static_store * const store)
{
	return csalt_store_error((const csalt_store * const)store);
}

ssize_t csalt_store_noop_read(csalt_static_store *_, void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

ssize_t csalt_store_noop_write(csalt_static_store *_, void *__, ssize_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

ssize_t csalt_store_noop_split(
	csalt_static_store *store,
	ssize_t _,
	ssize_t __,
	csalt_static_store_block_fn *block,
	void *param
)
{
	return block(store, param);
}

ssize_t csalt_store_noop_size(csalt_dynamic_store *_)
{
	(void)_;
	return -1;
}

ssize_t csalt_store_noop_resize(csalt_dynamic_store *_, ssize_t __)
{
	(void)_;
	(void)__;
	return -1;
}

static struct csalt_store_interface csalt_store_noop_implementation = {
	{
		csalt_store_noop_read,
		csalt_store_noop_write,
		csalt_store_noop_split,
	},
	csalt_store_noop_size,
	csalt_store_noop_resize,
};

extern csalt_store * const csalt_store_noop
	= &csalt_store_noop_implementation;
extern csalt_static_store * const csalt_static_store_noop
	= &csalt_store_noop_implementation.store_implementation;

