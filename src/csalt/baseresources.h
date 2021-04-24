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
struct csalt_resource_initialized_interface;
struct csalt_memory;

/**
 * \brief Represents a "wish to fulfill" a request for a resource.
 *
 * The csalt_resource interface only includes a single
 * function - csalt_resource_initialize() - which attempts
 * to initialize the resource, and returns either a
 * pointer to csalt_resource_initialized on success or
 * a NULL pointer on failure.
 *
 * To create custom structs which can manage resources,
 * use a struct csalt_resource_interface* as the first
 * member.
 *
 * Functions operating on resources take a csalt_resource
 * pointer; casting a pointer to your custom struct to
 * a (csalt_resource *) will allow you to use it in those
 * functions.
 *
 * \see csalt_resource_initialized
 */
typedef struct csalt_resource_interface *csalt_resource;

/**
 * \brief Represents a successfully initialized resource, which
 * 	implements the csalt_store interface and the
 * 	csalt_resource_deinit() function.
 *
 * To create custom structs which can operate on initialized
 * resources, use a struct csalt_resource_initialized_interface*
 * as the first member.
 *
 */
typedef struct csalt_resource_initialized_interface *csalt_resource_initialized;

/**
 * Function type for initializing the test on first use,
 * allows lazy evaluation of resources
 */
typedef csalt_resource_initialized *csalt_resource_init_fn(csalt_resource *resource);

typedef void csalt_resource_deinit_fn(csalt_resource_initialized *resource);

/**
 * \brief Interface definition for managed resources.
 *
 * Structs with a pointer-to-resource-interface
 * as their first member can be passed to resource
 * functions with a simple cast.
 *
 * This struct should not be instantiated instantly,
 * but instead be a member of a struct which is
 * itself set up with a function.
 */
struct csalt_resource_interface {
	csalt_resource_init_fn *init;
};

/**
 * \brief Interface definition for initialized resources.
 *
 * Structs with a pointer-to-initialized-resource-interface
 * as their first member can be passed to initialized resource
 * functions with a simple cast.
 *
 * Structs implementing this interface should only ever
 * be returned as a consiquence of passing a csalt_resource
 * to csalt_resource_init().
 */
struct csalt_resource_initialized_interface {
	struct csalt_store_interface parent;
	csalt_resource_deinit_fn *deinit;
};

/**
 * \brief Initializes a resource
 */
csalt_resource_initialized *csalt_resource_init(csalt_resource *);

/**
 * \brief Cleans up the resource. The resource is set
 * to an invalid value after run.
 */
void csalt_resource_deinit(csalt_resource_initialized *);

/**
 * A noop for init
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
	union {
		struct csalt_resource_initialized_interface *vtable;
		struct {
			struct csalt_memory parent;
			size_t size;
			size_t amount_written;
		};
	};
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
typedef int csalt_resource_block(csalt_resource_initialized *, csalt_store *);

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
