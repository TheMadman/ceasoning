#ifndef CSALTSTORES_H
#define CSALTSTORES_H

#include <sys/types.h>

#include "csalt/util.h"

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

struct csalt_store_interface;

/**
 * Any struct whos first member is a pointer to
 * a csalt_store_interface can be passed with a basic
 * cast to the virtual functions listed here.
 */
typedef struct csalt_store_interface *csalt_store;

/**
 * Function type for reading data from a store into a buffer
 */
typedef ssize_t csalt_store_read_fn(
	const csalt_store *store,
	void *buffer,
	size_t size
);

/**
 * Function type for writing data from a buffer into a store
 */
typedef ssize_t csalt_store_write_fn(
	csalt_store *store,
	const void *buffer,
	size_t size
);

/**
 * Type for a logic block to use inside csalt_store_split_fn
 * functions.
 */
typedef int csalt_store_block_fn(csalt_store *store, void *data);

/**
 * Function type for representing a sub-section of an
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
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * Returns the size of the store, if applicable, or 0
 * if not applicable to this store.
 *
 * Note that any store of size 0 can't be read to or
 * written from.
 */
typedef size_t csalt_store_size_fn(const csalt_store *store);

struct csalt_store_interface {
	csalt_store_read_fn *read;
	csalt_store_write_fn *write;
	csalt_store_size_fn *size;
	csalt_store_split_fn *split;
};

/**
 * Function for reading from a store into a buffer.
 *
 * Returns the amount of bytes actually read, or -1 on failure.
 */
ssize_t csalt_store_read(const csalt_store *store, void *buffer, size_t size);

/**
 * Function for writing to a store from a buffer.
 *
 * Returns the amount of bytes actually written, or -1 on failure.
 */
ssize_t csalt_store_write(csalt_store *store, const void *buffer, size_t size);

/**
 * Returns the current size of the given store.
 */
size_t csalt_store_size(const csalt_store *store);

/**
 * Provides the means to divide a store into a
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
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * This structure allows the transfer algorithm to run in
 * a non-blocking fashion. When the remaining is equal to
 * the total, the passed call-back is called, allowing you to
 * perform the same call in a loop (e.g. a render loop or
 * a spin-lock in case you want blocking behaviour).
 */
struct csalt_transfer {
	size_t total;
	size_t amount_completed;
};

/**
 * Creates a new struct csalt_transfer with the total
 * set to amount and the remaining set to 0.
 */
struct csalt_transfer csalt_transfer(size_t amount);

/**
 * Represents a function passed to csalt_store_transfer
 * to perform once the transfer has completed.
 */
typedef void csalt_transfer_complete_fn(csalt_store *dest);

/**
 * this function provides a convenient means to write data
 * from one store into another.
 *
 * If the transfer partially completes - for example, on
 * a non-blocking socket resource or similar - it returns
 * early, returning the actual amount of data transferred.
 *
 * Returns -1 on error.
 */
ssize_t csalt_store_transfer(
	struct csalt_transfer *data,
	csalt_store *to,
	csalt_store *from,
	csalt_transfer_complete_fn *callback
);

// null/noop interface
ssize_t csalt_store_null_read(
	const csalt_store *from,
	void *to,
	size_t size
);


ssize_t csalt_store_null_write(
	csalt_store *to,
	const void *from,
	size_t size
);

size_t csalt_store_null_size(const csalt_store *store);

int csalt_store_null_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * This interface represents a no-op interface, in the case of
 * errors.
 *
 * Reads and writes immediately return error values.
 * splits do nothing, and csalt_stores which
 * contain a pointer-to-null implementation can be considered
 * invalid or error return values.
 */
extern const struct csalt_store_interface *csalt_store_null_implementation;

// memory interface

ssize_t csalt_memory_read(
	const csalt_store *from,
	void *to,
	size_t size
);


ssize_t csalt_memory_write(
	csalt_store *to,
	const void *from,
	size_t size
);

size_t csalt_memory_size(const csalt_store *store);

int csalt_memory_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

extern const struct csalt_store_interface csalt_store_memory_implementation;

/**
 * A type representing a continuous block of memory.
 */
struct csalt_memory {
	const struct csalt_store_interface *vtable;
	char *begin;
	char *end;
};

/**
 * Creates a csalt_store_memory struct and sets up its
 * virtual call table.
 *
 * The end parameter should be the address immediately after
 * the last address containing the pointed-to data.
 */
struct csalt_memory csalt_store_memory_bounds(void *begin, void *end);

/**
 * Retrieves the pointer for direct access.
 * Should primarily be used for reading the data; simple,
 * safe writing is provided by data_store_transfer().
 */
void *csalt_store_memory_raw(struct csalt_memory *memory);

/**
 * Convenience macro for setting up a pointer-to-type
 */
#define csalt_store_memory_pointer(pointer) (csalt_store_memory_bounds((pointer), ((pointer) + 1)))

/**
 * Convenience macro for setting up an array-of-type
 */
#define csalt_store_memory_array(array) (csalt_store_memory_bounds((array), (&array[arrlength(array)])))


#endif // CSALTSTORES_H
