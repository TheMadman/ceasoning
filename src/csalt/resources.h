#ifndef SALTRESOURCES_H
#define SALTRESOURCES_H

#include <stddef.h>
#include <unistd.h>

#include <csalt/stores.h>

/**
 * \file
 * \brief Provides an interface for resources with lifetimes.
 *
 * This file provides both a common interface for resources which
 * require clean-up; a function to manage a resource automatically;
 * and some resource structs for common program resources like
 * heap memory and file descriptors.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct csalt_resource_interface;
struct csalt_memory;

/**
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
typedef void csalt_resource_init_fn(csalt_resource *resource);

/**
 * Function type for fetching a pointer to the resulting
 * resource.
 */
typedef void *csalt_resource_pointer_fn(csalt_resource *resource);

/**
 * Function type for writing data into a resource.
 *
 * Primarily used for csalt_transfer.
 */
typedef ssize_t csalt_resource_write_fn(
	csalt_resource *output,
	const void *input,
	size_t amount
);

/**
 * Function type for reading data out of a resource.
 *
 * Primarily used for csalt_transfer.
 */
typedef ssize_t csalt_resource_read_fn(
	const csalt_resource *input,
	void *output,
	size_t amount
);

/**
 * Function type for checking if resource allocation
 * was successful
 */
typedef char csalt_resource_valid_fn(const csalt_resource *);

/**
 * Function type for cleaning up a resource.
 */
typedef void csalt_deinit_fn(csalt_resource *);

/**
 * Interface definition for managed resources.
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 *
 * This struct should not be instantiated instantly,
 * but instead be a member of a struct which is
 * itself set up with a function.
 */
struct csalt_resource_interface {
	struct csalt_store_interface parent;
	csalt_resource_init_fn *init;
	csalt_resource_valid_fn *valid;
	csalt_deinit_fn *deinit;
};

/**
 * Initializes a resource
 */
void csalt_resource_init(csalt_resource *);

/**
 * Returns whether resource creation was successful.
 */
char csalt_resource_valid(const csalt_resource *);

/**
 * Cleans up the resource. The resource should
 * be considered invalid after run.
 */
void csalt_resource_deinit(csalt_resource *);

/**
 * A noop for init
 */
void csalt_noop_init(csalt_resource *_);

/**
 * A noop which always returns invalid
 */
char csalt_noop_valid(const csalt_resource *_);

/**
 * A noop for deinit
 */
void csalt_noop_deinit(csalt_resource *_);

/**
 * Represents a heap memory resource.
 *
 * Avoid using or modifying the members directly - simple code should
 * create this struct with csalt_memory_make and pass it to csalt_use,
 * or use it as a member for another resource.
 */
struct csalt_heap {
	const struct csalt_resource_interface * const vtable;
	size_t size;
	void *resource_pointer;
};

extern const struct csalt_heap csalt_null_heap;

/**
 * Initializes a csalt_memory resource. Uses malloc internally -
 * memory is allocated but not initialized.
 */
struct csalt_heap csalt_heap(size_t size);

/**
 * Function signature for blocks to pass to csalt_use.
 * The function should expect a pointer-to-resource as the
 * only argument, and return any result you wish to pass on,
 * or a null pointer.
 */
typedef struct csalt_heap csalt_resource_block(void *);

/**
 * Takes a pointer to resource struct and a code block,
 * passes the resource to the code block, cleans up the
 * resource and finally, returns the result of code_block.
 *
 * This function also checks if resource allocation was
 * successful and exits immediately when it failed.
 * Checking validity with csalt_resource_valid can be skipped
 * if the function passed in code_block returns an error value
 * on failure.
 *
 */
struct csalt_heap csalt_use(csalt_resource *resource, csalt_resource_block *code_block);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SALTRESOURCES_H
