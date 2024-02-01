/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2024   Marcus Harrison
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

#ifndef CSALT_RESOURCES_FILE_H
#define CSALT_RESOURCES_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

#include <csalt/resources.h>

/**
 * \file
 * \copydoc csalt_resource_file
 */

/**
 * \extends csalt_resource
 * \brief Represents a file on the file system.
 *
 * This file abstraction uses unbuffered reads/writes and
 * operates in a non-blocking mode by default.
 *
 * On csalt_resource_init(), a dynamic store is returned
 * which has an internal representation of its size.
 *
 * csalt_store_read(), csalt_store_write() and csalt_store_split()
 * are limited by the internally-stored size.
 *
 * csalt_store_size() updates the currently-stored size
 * from the filesystem and reports it.
 *
 * csalt_store_resize() attempts a truncate on the file. On
 * success, the new size is returned. On failure, the old
 * size is returned.
 */
struct csalt_resource_file {
	const struct csalt_dynamic_resource_interface *vtable;
	const char *path;
	int flags;
	int mode;
	struct csalt_store_file {
		const struct csalt_dynamic_store_interface *vtable;
		int fd;
		ssize_t begin;
		ssize_t end;
	} store;
};

/**
 * \public \memberof csalt_resource_file
 * \brief Constructor for a file resource.
 *
 * When constructed with this method, csalt_resource_init()
 * will open an existing file, or create a new file
 * if the file doesn't exist.
 *
 * \param path The file path
 * \param flags The flags to create the new file with, must
 * 	include one of O_RDONLY, O_WRONLY or O_RDWR
 * \param mode The mode to create a new file with
 *
 * \returns The new file resource
 */
struct csalt_resource_file csalt_resource_file(
	const char *path,
	int flags,
	int mode
);

/**
 * \public \memberof csalt_resource_file
 * \brief Constructor for a file resource.
 *
 * When constructed with this method, csalt_resource_init()
 * will create a new file, or return NULL if the file
 * already exists.
 *
 * \param path The file path
 * \param flags The flags to create the new file with, must
 * 	include one of O_RDONLY, O_WRONLY or O_RDWR
 * \param mode The mode to create a new file with
 *
 * \returns The new file resource
 */
struct csalt_resource_file csalt_resource_file_new(
	const char *path,
	int flags,
	int mode
);

/**
 * \public \memberof csalt_resource_file
 * \brief Constructor for a file resource.
 *
 * When constructed with this method, csalt_resource_init()
 * will open an existing file, or return NULL if the file
 * doesn't exist.
 *
 * \param path The file path
 * \param flags The flags to create the new file with, must
 * 	include one of O_RDONLY, O_WRONLY or O_RDWR
 *
 * \returns The new file resource
 */
struct csalt_resource_file csalt_resource_file_open(const char *path, int flags);

csalt_store *csalt_resource_file_init(csalt_resource *resource);
void csalt_resource_file_deinit(csalt_resource *resource);
ssize_t csalt_store_file_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
);
ssize_t csalt_store_file_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
);
int csalt_store_file_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
);
ssize_t csalt_store_file_size(csalt_store *store);
ssize_t csalt_store_file_resize(csalt_store *store, ssize_t new_size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_FILE_H
