#ifndef CSALT_BASERESOURCES_H
#define CSALT_BASERESOURCES_H

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
 * To create custom structs which can manage resources,
 * use a struct csalt_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a csalt_resource
 * pointer; casting a pointer to your custom struct to
 * a (csalt_resource *) will allow you to use it in those
 * functions.
 *
 * \see csalt_resource()
 */
typedef struct csalt_resource_interface *csalt_resource;

/**
 * Function type for initializing the test on first use,
 * allows lazy evaluation of resources
 */
typedef void csalt_resource_init_fn(csalt_resource *resource);

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
 * Cleans up the resource. The resource is set
 * to an invalid value after run.
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
 * \brief Represents a heap memory resource.
 *
 * Avoid using or modifying the members directly - simple code should
 * create this struct with csalt_memory_make and pass it to csalt_resource_use,
 * or use it as a member for another resource.
 *
 * This struct does not allow reading back more data than has been written to
 * it with csalt_store_write() or csalt_store_transfer(). This helps to
 * prevent uninitialized memory. It also allows using a heap as a caching
 * element in csalt_store_fallback with ease.
 *
 * \see csalt_heap()
 * \see csalt_heap_lazy()
 */
struct csalt_heap {
	struct csalt_memory parent;
	size_t size;
	size_t amount_written;
};

extern const struct csalt_heap csalt_null_heap;

/**
 * \brief Initializes a csalt_heap resource. Uses malloc internally -
 * memory is allocated but not initialized.
 *
 * This function immediately allocates the heap, allowing you
 * to manually check with csalt_resource_valid() that the allocation
 * was successful. It must be freed with csalt_resource_deinit()
 * once finished.
 *
 * Primarily useful for the store functions. Consider
 * using csalt_heap_lazy() and having the heap managed for you
 * automatically by csalt_resource_use().
 *
 * \see csalt_heap_lazy()
 */
struct csalt_heap csalt_heap(size_t size);

/**
 * \brief This function creates a heap resource for later initialization.
 *
 * This is useful behaviour for abstract data-types which automatically
 * perform initialization on their members.
 *
 * This does not immediately attempt to allocate memory on the heap.
 * If you need the pointer for immediate use, as with the typical
 * malloc -> check -> use -> free workflow, use the csalt_heap()
 * function instead.
 *
 * \see csalt_heap()
 */
struct csalt_heap csalt_heap_lazy(size_t size);

/**
 * \brief Gives immediate access to the raw pointer.
 * 
 * This function should only really be used for reading data from
 * the resulting pointer. Safe transfer with other stores
 * can be done with csalt_store_transfer(), and safe copying
 * can be done with csalt_store_read().
 */
void *csalt_resource_heap_raw(const struct csalt_heap *heap);

/**
 * \brief Function signature for blocks to pass to csalt_resource_use.
 *
 * The function should expect a pointer-to-resource and a
 * pointer-to-store as arguments, with the store acting as an
 * out parameter, and return a value useful for error checking
 * in your code.
 *
 * Note that csalt_resource_use returns -1 if there was an error
 * initializing the resource.
 *
 * \see csalt_resource_use()
 */
typedef int csalt_resource_block(csalt_resource *, csalt_store *);

/**
 * \brief Manages a resource lifecycle and executes the given
 * function.
 *
 * Takes a pointer to resource struct, a code block and
 * an out parameter implementing the csalt_store interface,
 * passes the resource and out parameter to the code block,
 * cleans up the resource and finally, returns the result of
 * code_block.
 *
 * This function also checks if resource allocation was
 * successful and returns -1 if it failed.
 * Checking validity with csalt_resource_valid can be skipped
 * if the function passed in code_block returns an error value
 * on failure.
 */
int csalt_resource_use(
	csalt_resource *resource,
	csalt_resource_block *code_block,
	csalt_store *out
);

/**
 * Provides a shorthand for castto(csalt_resource *, (param))
 */
#define csalt_resource(param) castto(csalt_resource *, (param))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_BASERESOURCES_H
