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

#include <csalt/fileresource.h>

#include <unistd.h>
#include <fcntl.h>
#include <csalt/util.h>
#include <errno.h>

struct csalt_resource_interface csalt_resource_file_interface = {
	csalt_resource_file_init,
	csalt_resource_file_deinit,
};

int csalt_resource_file_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_resource_file_initialized *file = castto(file, store);
	struct csalt_resource_file_initialized result = *file;
	result.end = result.begin + end;
	result.begin += begin;
	return block(castto(csalt_store *, &result), data);
}

csalt_store *csalt_resource_file_init(csalt_resource *resource)
{
	struct csalt_resource_file *file = castto(file, resource);

	// Some open flags break the API for reading/writing
	int banned_flags = O_APPEND;

	// on the other hand, some flags are implied by the
	// operation of this library, primarily O_NONBLOCK
	int implied_flags = O_NONBLOCK;

	int fd = open(file->filename, (implied_flags | file->flags) & ~banned_flags, file->mode);
	if (fd > -1) {
		file->file = csalt_store_file_descriptor(fd);
		return (csalt_store *)&file->file;
	} else {
		return 0;
	}
}

void csalt_resource_file_deinit(csalt_resource *resource)
{
	struct csalt_resource_file *file = (struct csalt_resource_file *)resource;
	close(file->file.fd);
	file->file.fd = -1;
}

struct csalt_resource_file csalt_resource_file(const char *path, int flags)
{
	struct csalt_resource_file file = {
		&csalt_resource_file_interface,
		path,
		flags,
		0,
		csalt_store_file_descriptor(-1),
	};
	return file;
}

struct csalt_resource_file csalt_resource_create_file(const char *path, int flags, int mode)
{
	struct csalt_resource_file result = csalt_resource_file(path, flags | O_CREAT);
	result.mode = mode;
	return result;
}

