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

#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include <csalt/platform/init.h>

#include "stores.h"
#include "baseresources.h"

/**
 * \file
 * \brief Provides a wrapper around on-disk files for use
 * with csalt_resource algorithms.
 *
 * This file provides a concrete file type, based on file
 * descriptors, to on-disk files specified by a string pathname.
 *
 * Allows the creation of new files and opening of existing
 * files.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Struct representing a file resource supporting lazy-loading.
 *
 * Do not attempt to initialize directly; instead, use the
 * csalt_resource_file function.
 *
 * Files opened with this library are automatically set to non-blocking,
 * and if a read/write operation would block, the csalt read/write functions
 * simply return 0 bytes read/written. This allows the generic
 * read/write functions to separate zero bytes transferred from an
 * error arising from the resource itself.
 *
 * \see csalt_resource_file()
 */
struct csalt_resource_file {
	const struct csalt_resource_interface *vtable;
	const char *filename;
	int flags;
	int mode;
	struct csalt_store_file_descriptor file;
};

ssize_t csalt_resource_file_read(csalt_store *store, void *buffer, ssize_t size);

ssize_t csalt_resource_file_write(csalt_store *store, const void *buffer, ssize_t size);

ssize_t csalt_resource_file_size(csalt_store *store);

int csalt_resource_file_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

csalt_store *csalt_resource_file_init(csalt_resource *resource);

void csalt_resource_file_deinit(csalt_resource *resource);

/**
 * Creates a file resource with the given path and flags. The flags
 * are those passed to unistd.h's open function.
 *
 * Some flags interfere with normal operation of the API, specifically:
 * - O_APPEND
 *
 * Others will cause some functions to error, such as opening a file read-only
 * causing csalt_resource_file_write to error. The file resource will
 * still be usable with the features it does support.
 *
 * Some flags are implicit with the expected functionality of the library,
 * specifically:
 * - O_NONBLOCK
 *
 * These need not be specified and are set automatically.
 *
 * \see csalt_resource_file
 */
struct csalt_resource_file csalt_resource_file(const char *path, int flags);

/**
 * Sets up a file resource for creation and opening.
 *
 * Notes:
 * - creating the file is only attempted once the resource is initialized by
 *   being passed to csalt_resource_init() or csalt_use().
 * - The O_CREAT flag is implied and needs not be passed into the flags parameter.
 */
struct csalt_resource_file csalt_resource_create_file(const char *path, int flags, int mode);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILERESOURCE_H
