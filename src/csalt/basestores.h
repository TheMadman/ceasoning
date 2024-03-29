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

#ifndef CSALTSTORES_H
#define CSALTSTORES_H

#include <csalt/platform/init.h>

#include <csalt/util.h>

/**
 * \file
 * \brief This file defines interfaces for anything
 * which data can be written to or read from.
 *
 * These interfaces are intended to also represent
 * data stores which do not need to be acquired or
 * released, such as application global memory and stack
 * frame memory.
 */

/**
 * \brief Any struct whos first member is a pointer to
 * a csalt_store_interface can be passed with a basic
 * cast to the virtual functions listed here.
 */
typedef const struct csalt_store_interface * const csalt_store;

/**
 * \brief Function type for reading data from a store into a buffer
 */
typedef ssize_t csalt_store_read_fn(
	csalt_store *store,
	void *buffer,
	ssize_t size
);

/**
 * \brief Function type for writing data from a buffer into a store
 */
typedef ssize_t csalt_store_write_fn(
	csalt_store *store,
	const void *buffer,
	ssize_t size
);

/**
 * \brief Type for a logic block to use inside csalt_store_split_fn
 * functions.
 */
typedef int csalt_store_block_fn(csalt_store *store, void *data);

/**
 * \brief Function type for representing a sub-section of an
 * existing store as a new store, and performing an
 * action on the result.
 *
 * The last thing this function should do is return a
 * call to block(result, data) on success, or return an
 * error code on failure.
 *
 * \see csalt_store_split
 *
 */
typedef int csalt_store_split_fn(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief Returns the size of the store, if applicable, or 0
 * if not applicable to this store.
 */
typedef ssize_t csalt_store_size_fn(csalt_store *store);

struct csalt_store_interface {
	csalt_store_read_fn *read;
	csalt_store_write_fn *write;
	csalt_store_size_fn *size;
	csalt_store_split_fn *split;
};

/**
 * \brief Function for reading from a store into a buffer.
 *
 * Returns the amount of bytes actually read, or -1 on failure.
 *
 * \sa csalt_read()
 */
ssize_t csalt_store_read(csalt_store *store, void *buffer, ssize_t size);

/**
 * \brief Function for writing to a store from a buffer.
 *
 * Returns the amount of bytes actually written, or -1 on failure.
 *
 * \sa csalt_write()
 */
ssize_t csalt_store_write(csalt_store *store, const void *buffer, ssize_t size);

/**
 * \brief Returns the current size of the given store.
 *
 * Generally, calling csalt_store_size before calling one of csalt_store_read
 * or csalt_store_write is racy: this is true of most underlying stores,
 * including files, shared memory or sockets. Your application can request
 * the size, then the store's size can be changed without your application
 * knowing by a write by another process altogether. Therefore, you should only
 * use this pattern if you have good reason to believe that the store will not
 * arbitrarily change size - for example, because you control the file or
 * the endpoint of the network socket.
 *
 * The much safer approach is to simply attempt to csalt_store_read or
 * csalt_store_write from the store and check the return values for amount
 * read/written and error values. If you are constructing stores from user
 * input, you should try to formulate your application to use csalt_store_size
 * as little as possible.
 */
ssize_t csalt_store_size(csalt_store *store);

/**
 * \brief Provides the means to divide a store into a
 * sub-section and perform an operation on the result.
 *
 * One of the limitations of this function is to prevent
 * performing system calls (specifically, allocating heap
 * memory), unless used with stores which implement those
 * system calls. This limitation prevents some more conventional
 * approaches, including returning a pointer-to-store in
 * a nicely reentrant way; or using an out parameter pointer
 * if the caller does not know the structure of the returned
 * type (because it relies on polymorphic behaviour).
 *
 * For this reason, the use of this function is rather
 * unconventional - the first parameters are as expected:
 * the store you want to sub-section, and how you want to
 * sub-section it. The last two parameters are the logic
 * block to be performed on the resulting sub-section, and
 * additional data to pass to the logic block, for persisting
 * data after the return. This can be used in a
 * pseudo-recursive way to perform complex processing without
 * the overhead of additional system calls or the complexity
 * of resource (de-)allocation.
 *
 * The return value is the return value of the block parameter
 * and can be used for error handling.
 */
int csalt_store_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief This structure allows the transfer algorithm to run in
 * a non-blocking fashion.
 *
 * When the remaining is equal to the total, the passed call-back is called,
 * allowing you to perform the same call in a loop (e.g. a render loop or a 
 * spin-lock in case you want blocking behaviour).
 *
 * \see csalt_progress()
 */
struct csalt_progress {
	ssize_t total;
	ssize_t amount_completed;
};

/**
 * \brief Creates a new struct csalt_progress with the total
 * set to amount and the remaining set to 0.
 */
struct csalt_progress csalt_progress(ssize_t amount);

/**
 * \brief This function returns truthy if progress is finished, false
 * otherwise
 */
int csalt_progress_complete(struct csalt_progress *progress);

/**
 * \brief Represents a function passed to csalt_store_transfer
 * to perform once the transfer has completed.
 */
typedef void csalt_transfer_complete_fn(csalt_store *dest);

/**
 * \brief This function provides a convenient means to write data
 * from one store into another.
 *
 * If the transfer partially completes - for example, on
 * a non-blocking socket resource or similar - it returns
 * early, returning the total data transferred so far.
 *
 * Returns -1 on error.
 */
ssize_t csalt_store_transfer(
	struct csalt_progress *data,
	csalt_store *from,
	csalt_store *to,
	csalt_transfer_complete_fn *callback
);

// null/noop interface
ssize_t csalt_store_null_read(
	csalt_store *from,
	void *to,
	ssize_t size
);

ssize_t csalt_store_null_write(
	csalt_store *to,
	const void *from,
	ssize_t size
);

ssize_t csalt_store_null_size(csalt_store *store);

int csalt_store_null_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief This interface represents a no-op interface, in the case of
 * errors.
 *
 * Reads and writes immediately return error values.
 * splits do nothing, and csalt_stores which
 * contain a pointer-to-null implementation can be considered
 * invalid or error return values.
 */
extern const struct csalt_store_interface *csalt_store_null_implementation;

extern csalt_store *csalt_store_null;

// memory interface

ssize_t csalt_memory_read(
	csalt_store *from,
	void *to,
	ssize_t size
);

ssize_t csalt_memory_write(
	csalt_store *to,
	const void *from,
	ssize_t size
);

ssize_t csalt_memory_size(csalt_store *store);

int csalt_memory_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

extern const struct csalt_store_interface csalt_memory_implementation;

/**
 * \brief A type representing a continuous block of memory.
 *
 * Use this store to represent an already-managed block of memory,
 * such as function stack or application-global memory.
 *
 * For heap memory, csalt_heap provides a resource which can
 * be managed for you by passing it to csalt_resource_use().
 *
 * For const memory, consider csalt_cmemory instead.
 *
 * \sa csalt_cmemory
 * \sa csalt_memory_pointer()
 * \sa csalt_memory_array()
 * \sa csalt_memory_bounds()
 * \sa csalt_heap
 */
struct csalt_memory {
	const struct csalt_store_interface *vtable;
	char *begin;
	char *end;
};

/**
 * Creates a csalt_memory struct and sets up its
 * virtual call table.
 *
 * The end parameter should be the address immediately after
 * the last address containing the pointed-to data.
 */
struct csalt_memory csalt_memory_bounds(void *begin, void *end);

/**
 * Retrieves the pointer for direct access.
 * Should primarily be used for reading the data; simple,
 * safe writing is provided by data_store_transfer().
 */
void *csalt_memory_raw(const struct csalt_memory *memory);

/**
 * \brief Convenience macro for setting up a pointer-to-type
 */
#define csalt_memory_pointer(pointer) (csalt_memory_bounds((pointer), (pointer) + 1))

/**
 * \brief Convenience macro for setting up an array-of-type
 */
#define csalt_memory_array(array) (csalt_memory_bounds((array), (&array[arrlength(array)])))

ssize_t csalt_cmemory_read(
	csalt_store *from,
	void *to,
	ssize_t size
);

ssize_t csalt_cmemory_size(csalt_store *store);

int csalt_cmemory_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief A type representing a continuous block of read-only
 * 	memory.
 *
 * This type is similar to the csalt_memory type, except that it
 * accepts const-pointers and disables writing.
 *
 * \sa csalt_memory
 * \sa csalt_cmemory_pointer()
 * \sa csalt_cmemory_array()
 * \sa csalt_cmemory_bounds()
 */
struct csalt_cmemory {
	const struct csalt_store_interface *vtable;
	const char *begin;
	const char *end;
};

/**
 * \brief Constructs a csalt_cmemory with write operations
 * 	disabled.
 */
struct csalt_cmemory csalt_cmemory_bounds(const void *begin, const void *end);

/**
 * \brief Retrieves a raw pointer into a csalt_cmemory.
 */
const void *csalt_cmemory_raw(const struct csalt_cmemory *memory);

/**
 * \brief Convenience macro for setting up a pointer-to-type
 */
#define csalt_cmemory_pointer(pointer) (csalt_cmemory_bounds((pointer), (pointer) + 1))

/**
 * \brief Convenience macro for setting up an array-of-type
 */
#define csalt_cmemory_array(array) (csalt_cmemory_bounds((array), arrend(array)))

/**
 * \brief Convenience macro for writing a stack value to a store.
 *
 * This macro takes the given value and attempts to write it to the
 * store, returning the amount written.
 *
 * Example usage:
 *
 * \code
 *
 * int my_func(csalt_store *store, void *param)
 * {
 * 	int triangle_points[2][3] = {
 * 		0, 0,
 * 		0, 1,
 * 		1, 1,
 * 	};
 *
 * 	ssize_t written = csalt_write(store, triangle_points);
 * 	if (written < 0) {
 * 		// handle error
 * 	}
 * 	if (written < sizeof(triangle_points)) {
 * 		// handle partial write
 * 	}
 * }
 *
 * \endcode
 */
#define csalt_write(store, value) \
	csalt_store_write(csalt_store(store), &value, sizeof(value));

/**
 * \brief Convenience macro for reading a stack value from a store.
 *
 * This macro takes the given value and attempts to read from the given
 * store into it, returning the amount read.
 *
 * Example usage:
 *
 * \code
 *
 * int my_func(csalt_store *store, void *param)
 * {
 * 	char magic_number[8] = { 0 };
 *
 * 	ssize_t read = csalt_read(store, magic_number);
 *
 * 	if (read < 0) {
 * 		// handle error
 * 	}
 * 	if (read < sizeof(magic_number)) {
 *		// handle partial read
 *	}
 *
 *	const char expected[] = "\x89" "\x50" "\x4e" "\x47"
 *		"\x0d" "\x0a" "\x1a" "\x0a";
 *	if (strncmp(expected, magic_number, sizeof(magic_number))) {
 *		// handle not PNG file
 *	}
 * }
 *
 * \endcode
 */
#define csalt_read(store, value) \
	csalt_store_read(csalt_store(store), &value, sizeof(value))

/**
 * Convenience macro for casting to store pointer
 */
#define csalt_store(param) (csalt_store *)(param)

/**
 * \brief Store representing a file descriptor.
 *
 * This store is used for file descriptors which are 
 * available by default, including stdin, stdout and stderr.
 *
 * Also useful for resources which manage file descriptors.
 *
 * \sa csalt_store_file_descriptor()
 */
struct csalt_store_file_descriptor {
	const struct csalt_store_interface *vtable;
	int fd;
	ssize_t (*reader)(
		struct csalt_store_file_descriptor *,
		void *,
		ssize_t
	);
	ssize_t (*writer)(
		struct csalt_store_file_descriptor *,
		const void *,
		ssize_t
	);
	ssize_t begin;
	ssize_t end;
};

/**
 * \brief Constructor for csalt_store_file_descriptor.
 */
struct csalt_store_file_descriptor csalt_store_file_descriptor(int fd);

ssize_t csalt_store_file_descriptor_read(
	csalt_store *from,
	void *to,
	ssize_t bytes
);

ssize_t csalt_store_file_descriptor_write(
	csalt_store *to,
	const void *from,
	ssize_t bytes
);

ssize_t csalt_store_file_descriptor_size(csalt_store *store);

int csalt_store_file_descriptor_split(
	csalt_store *file,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

#endif // CSALTSTORES_H
