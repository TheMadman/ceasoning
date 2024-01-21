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

#include "csalt/resources/file.h"

#include <unistd.h>
#include <fcntl.h>
#include <csalt/util.h>
#include <errno.h>

#include "csalt/util.h"

typedef struct csalt_resource_file file_t;
typedef struct csalt_store_file file_store_t;

static const struct csalt_resource_interface impl = {
	csalt_resource_file_init,
	csalt_resource_file_deinit,
};

static const struct csalt_dynamic_store_interface store_impl = {
	{
		csalt_store_file_read,
		csalt_store_file_write,
		csalt_store_file_split,
	},
	csalt_store_file_size,
	csalt_store_file_resize,
};

static struct csalt_resource_file construct(
	const char *path,
	int mode,
	int flags
)
{
	return (file_t) {
		.vtable = &impl,
		.path = path,
		.flags = flags,
		.mode = mode,
		.store = {
			.vtable = &store_impl,
			.fd = -1,
			.begin = 0,
			.end = 0,
		},
	};
}

struct csalt_resource_file csalt_resource_file(
	const char *path,
	int flags,
	int mode
)
{
	return construct(path, mode, O_NONBLOCK | O_CREAT | flags);
}

struct csalt_resource_file csalt_resource_file_new(
	const char *path,
	int flags,
	int mode
)
{
	return construct(path, mode, O_NONBLOCK | O_CREAT | O_EXCL | flags);
}

struct csalt_resource_file csalt_resource_file_open(const char *path, int flags)
{
	return construct(path, 0, O_NONBLOCK | flags);
}

csalt_store *csalt_resource_file_init(csalt_resource *resource)
{
	file_t *file = (file_t *)resource;
	file->store.fd = open(
		file->path,
		file->flags,
		file->mode);

	if (file->store.fd == -1)
		return NULL;

	csalt_store_size(&file->store.vtable);
	return &file->store.vtable;
}

void csalt_resource_file_deinit(csalt_resource *resource)
{
	file_t *file = (file_t *)resource;
	close(file->store.fd);
	file->store.fd = -1;
}

ssize_t csalt_store_file_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	file_store_t *file = (file_store_t *)store;
	amount = csalt_max(
		amount,
		file->end - file->begin);
	return pread(
		file->fd,
		buffer,
		(size_t)amount,
		file->begin);
}

ssize_t csalt_store_file_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	file_store_t *file = (file_store_t *)store;
	amount = csalt_max(
		amount,
		file->end - file->begin);
	return pwrite(
		file->fd,
		buffer,
		(size_t)amount,
		file->begin);
}

static ssize_t new_offset(file_store_t file, ssize_t offset)
{
	return csalt_max(
		0,
		csalt_min(
			file.begin + offset,
			file.end));
}

int csalt_store_file_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	file_store_t *file = (file_store_t *)store;

	file_store_t new_file = *file;
	new_file.begin = new_offset(*file, begin);
	new_file.end = new_offset(*file, end);

	return block((csalt_static_store *)&new_file, param);
}

ssize_t csalt_store_file_size(csalt_store *store)
{
	file_store_t *file = (file_store_t *)store;
	lseek(file->fd, 0, SEEK_END);
	return file->end - file->begin;
}

ssize_t csalt_store_file_resize(csalt_store *store, ssize_t new_size)
{
	file_store_t *file = (file_store_t *)store;
	if (ftruncate(file->fd, new_size))
		return file->end - file->begin;
	file->end = new_size;
	return new_size;
}

