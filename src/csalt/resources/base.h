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

#ifndef CSALT_RESOURCES_BASE_H
#define CSALT_RESOURCES_BASE_H

#include <csalt/platform/init.h>

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
 * use a struct csalt_dynamic_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a csalt_resource
 * pointer; casting a pointer to your custom struct to
 * a (csalt_resource *) will allow you to use it in those
 * functions.
 */
typedef const struct csalt_dynamic_resource_interface *const csalt_resource;

/**
 * \brief Identical to a csalt_resource, but returns a static
 * 	store.
 *
 * While it is safe to pass a csalt_resource to csalt_static_resource_init(),
 * it is **not** safe to pass a csalt_static_resource to csalt_resource_init()
 * and use the csalt_store_size() or csalt_store_resize() functions on
 * the returned result.
 *
 * \sa csalt_resource
 */
typedef const struct csalt_static_resource_interface *const csalt_static_resource;

/**
 * Function type for initializing the test on first use,
 * allows lazy evaluation of resources
 */
typedef csalt_store *csalt_resource_init_fn(csalt_resource *resource);

typedef csalt_static_store *csalt_static_resource_init_fn(csalt_static_resource *);

typedef void csalt_resource_deinit_fn(csalt_resource *resource);

/**
 * \brief Interface definition for managed resources.
 *
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 */
struct csalt_dynamic_resource_interface {
	csalt_resource_init_fn *init;
	csalt_resource_deinit_fn *deinit;
};

/**
 * \brief Interface definition for managed resources which
 * 	return static (non-resizable) stores.
 *
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 */
struct csalt_static_resource_interface {
	csalt_static_resource_init_fn *init;
	csalt_resource_deinit_fn *deinit;
};

/**
 * \brief Initializes a resource, returning a csalt_store.
 */
csalt_store *csalt_resource_init(csalt_resource *);

/**
 * \brief Initializes a resource, returning a csalt_static_store.
 */
csalt_static_store *csalt_static_resource_init(csalt_static_resource *);

/**
 * \brief Cleans up the resource. The resource is set
 * to an invalid value after run.
 *
 * This function is used for csalt_static_resources as well.
 */
void csalt_resource_deinit(csalt_resource *);

typedef int csalt_store_block_fn(csalt_store *, void *);

/**
 * \brief Manages a resource lifecycle and executes the given
 * 	function.
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
	void *param);

int csalt_static_resource_use(
	csalt_static_resource *resource,
	csalt_static_store_block_fn *code_block,
	void *param);

/**
 * \brief Macro for managing a resource lifecycle, executing the
 * 	given function.
 *
 * return_destination is the name of a variable which will contain
 * the return value of running generic_function. It is not modified
 * if the resource initialization failed.
 * return_destination must be passed, and generic_function must
 * return a value of the same type as return_destination, even if
 * it is a dummy/unused value.
 *
 * resource is a pointer to a csalt_resource struct to be managed.
 *
 * generic_function is a function with an arbitrary return type,
 * taking as its first argument a pointer to a csalt_store struct,
 * plus arbitrary additional arguments.
 * generic_function must return a value of the same type as
 * return_destination, even if it is a dummy/unused value.
 *
 * After generic_function, you may pass any number of arguments,
 * which will be passed to generic_function after the first
 * csalt_store pointer argument.
 */
#define CSALT_USE(return_destination, resource, generic_function, ...) \
	do { \
		csalt_store \
			*csalt_use_init_r_ = csalt_resource_init( \
				(csalt_resource *)resource \
			); \
		if (!csalt_use_init_r_) \
			break; \
		return_destination = generic_function( \
			csalt_use_init_r_, \
			__VA_ARGS__ \
		); \
		csalt_resource_deinit((csalt_resource *)resource); \
	} while (0)

/**
 * Provides a shorthand for ((csalt_resource *)(param))
 */
#define csalt_resource(param) ((csalt_resource *)(param))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_RESOURCES_BASE_H
