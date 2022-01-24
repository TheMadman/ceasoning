#ifndef CSALT_BASERESOURCES_H
#define CSALT_BASERESOURCES_H

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

#include <stddef.h>
#include <unistd.h>

#include <csalt/stores.h>

/**
 * \file
 * \brief Provides an interface for resources with lifetimes.
 *
 * This file provides both a common interface for resources which
 * require clean-up; a function to manage a resource automatically;
 * and a resource for managing heap memory.
 *
 * Note that resources do not _necessarily_ do bounds checking - 
 * for some resources, writing beyond the end of them is a valid
 * operation for appending data. The heap resource does a primitive
 * test that the size attribute is smaller than the current heap
 * block.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct csalt_resource_interface;
struct csalt_memory;

/**
 * \brief Represents a "wish to fulfill" a request for a resource.
 *
 * The csalt_resource interface includes a function for
 * initializing a store and a function for deinitializing
 * a store. csalt_resource_init() attempts to fulfill a
 * request for a resource from the system, returning a
 * pointer to a csalt_store on success or a null pointer
 * on failure.
 *
 * csalt_resource_deinit() takes a resource which has already
 * had csalt_resource_init() called on it and returns the
 * resource to the operating system. The csalt_store which
 * was returned by csalt_resource_init() is invalid, and
 * attempting to use it is undefined behaviour (for obvious
 * reasons - writing to a heap after it's freed, or a file
 * descriptor after it has closed, etc.)
 *
 * The recommended way to manage resource life-cycles is
 * by passing them to the csalt_resource_use() function,
 * which will initialize the resource, test the store,
 * pass it to your function and deinitialize the resource
 * after.
 *
 * To create custom structs which can manage resources,
 * use a struct csalt_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a csalt_resource
 * pointer; casting a pointer to your custom struct to
 * a (csalt_resource *) will allow you to use it in those
 * functions.
 */
typedef struct csalt_resource_interface *csalt_resource;

/**
 * Function type for initializing the test on first use,
 * allows lazy evaluation of resources
 */
typedef csalt_store *csalt_resource_init_fn(csalt_resource *resource);

typedef void csalt_resource_deinit_fn(csalt_resource *resource);

/**
 * \brief Interface definition for managed resources.
 *
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 */
struct csalt_resource_interface {
	csalt_resource_init_fn *init;
	csalt_resource_deinit_fn *deinit;
};

/**
 * \brief Initializes a resource
 */
csalt_store *csalt_resource_init(csalt_resource *);

/**
 * \brief Cleans up the resource. The resource is set
 * to an invalid value after run.
 */
void csalt_resource_deinit(csalt_resource *);

/**
 * \brief A noop for init, returning null
 */
void csalt_noop_init(csalt_resource *_);

/**
 * A noop for deinit
 */
void csalt_noop_deinit(csalt_resource *_);

/**
 * \brief Represents a heap memory resource.
 *
 * A pointer to this is returned when csalt_resource_init() is called
 * on a csalt_heap. Use csalt_resource_heap_raw() to retrieve a raw
 * pointer.
 *
 * This struct does not allow reading back more data than has been written to
 * it with csalt_store_write() or csalt_store_transfer(). This helps to
 * prevent reading uninitialized memory. It also allows using a heap as a caching
 * element in csalt_store_fallback with ease.
 */
struct csalt_heap_initialized {
	struct csalt_store_interface *vtable;
	struct csalt_memory memory;
	size_t size;
	size_t amount_written;
};

/**
 * \brief Represents a heap memory resource.
 *
 * Avoid using or modifying the members directly - simple code should
 * create this struct with csalt_heap() and pass it to csalt_resource_use,
 * or use it as a member for another resource.
 *
 * \see csalt_heap()
 */
struct csalt_heap {
	struct csalt_resource_interface *vtable;
	struct csalt_heap_initialized heap;
};

/**
 * \brief Initializes a csalt_heap resource. Uses malloc internally -
 * memory is allocated but not initialized.
 *
 */
struct csalt_heap csalt_heap(size_t size);

/**
 * \brief Gives immediate access to the raw pointer.
 * 
 * This function should only really be used for reading data from
 * the resulting pointer. Safe transfer with other stores
 * can be done with csalt_store_transfer(), and safe copying
 * can be done with csalt_store_read().
 */
void *csalt_resource_heap_raw(const struct csalt_heap_initialized *heap);

/**
 * \brief A type representing an initialized vector, implementing the
 * 	csalt_store interface.
 *
 * Writes and splits past the end of this type will attempt to resize the
 * pointed-to heap memory before writing.
 *
 * Initialization and clean-up are performed by csalt_resource_vector, which
 * is how you should use this type. Otherwise, you must perform clean-up of
 * the internal pointer yourself, which you should avoid.
 */
struct csalt_resource_vector_initialized {
	struct csalt_store_interface *vtable;
	
	/**
	 * \brief This pointer is the pointer to the initialized heap memory,
	 * as returned by malloc() or realloc(), and is the member that
	 * needs free() called on it.
	 *
	 * Again, you should only use this type via
	 * csalt_resource_vector, which will do the initialization and
	 * clean-up for you when passed to csalt_resource_use().
	 */
	void *original_pointer;

	/**
	 * \brief This pointer is the pointer to the end of the initialized
	 * heap memory.
	 *
	 * This is the value that's checked for when to expand the vector.
	 */
	void *original_end;

	/**
	 * \brief Index from original_pointer representing the
	 * beginning of the split
	 */
	size_t begin;

	/**
	 * \brief Index from original_pointer representing the
	 * end of the split.
	 */
	size_t end;

	/**
	 * \brief Stores the amount of data written, to prevent
	 * reading uninitialized values.
	 *
	 * If you split/write after the beginning, then read from the beginning,
	 * the memory before your write will be initialized to 0.
	 */
	size_t amount_written;
};

/**
 * \brief Manages heap memory which can expand with csalt_store_write() calls.
 *
 * This type is similar to csalt_heap, except that writing past the end of
 * the heap memory attempts a reallocation and copy. The newly reallocated
 * size attempted is simply twice the previous size.
 */
struct csalt_resource_vector {
	struct csalt_resource_interface *vtable;
	size_t size;
	struct csalt_resource_vector_initialized vector;
};

/**
 * \brief Constructs a vector with an initial buffer size.
 *
 * Passing 0 produces a vector with a default buffer size.
 */
struct csalt_resource_vector csalt_resource_vector(size_t initial_size);

/**
 * \brief Manages a resource lifecycle and executes the given
 * function.
 *
 * Takes a pointer to resource struct, a code block and
 * an optional parameter, passes the resource and parameter
 * to the code block, cleans up the resource and finally,
 * returns the result of code_block.
 *
 * csalt_resource_use itself will return -1 if the resource
 * allocation failed. Otherwise, it will return the return value
 * of the passed block.
 */
int csalt_resource_use(
	csalt_resource *resource,
	csalt_store_block_fn *code_block,
	void *out
);

/**
 * Provides a shorthand for castto(csalt_resource *, (param))
 */
#define csalt_resource(param) castto(csalt_resource *, (param))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_BASERESOURCES_H
