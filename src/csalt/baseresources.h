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
