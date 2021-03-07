#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include <sys/types.h>

#include "stores.h"
#include "baseresources.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Struct representing a file resource supporting lazy-loading.
 * Do not attempt to initialize directly; instead, use the
 * csalt_resource_file function.
 *
 * \see csalt_resource_file
 */
struct csalt_resource_file {
	const struct csalt_resource_interface *vtable;
	const char *filename;
	int flags;
	int fd;
	size_t begin;
	size_t end;
};

ssize_t csalt_resource_file_read(const csalt_store *store, void *buffer, size_t size);

ssize_t csalt_resource_file_write(csalt_store *store, const void *buffer, size_t size);

size_t csalt_resource_file_size(const csalt_store *store);

int csalt_resource_file_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

void csalt_resource_file_init(csalt_resource *resource);

char csalt_resource_file_valid(const csalt_resource *resource);

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
 */
struct csalt_resource_file cslt_resource_file(const char *path, int flags);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILERESOURCE_H
