#ifndef CSALT_COMPOSITERESOURCES_H
#define CSALT_COMPOSITERESOURCES_H

/**
 * \file
 * \brief This file defines abstract resources, which
 * allow you to define relationships between resources.
 *
 * This includes the csalt_resource_list resource, which
 * allows you to treat any number of resources as a single
 * resource safely
 */

#include <csalt/baseresources.h>
#include <csalt/compositestores.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This type allows storing a pair of resources and treating them
 * 	as a single resource.
 *
 * This type can be used to represent more complex abstract datatypes; see
 * the documentation for csalt_store_pair for examples. Either resource may
 * be null.
 *
 * Constructors are available for constructing pairs and lists of pairs.
 *
 * csalt_resource_init() attempts to initialize the first resource first. If
 * that fails, the whole resource fails. If it succeeds, the second resource
 * is attempted. If that fails, the first resource is uninitialized, then the
 * pair returns failure.
 *
 * If both resources succeed in initializing, this pair passes a pointer to a
 * csalt_store_pair to the callback.
 *
 * csalt_resource_deinit() simply calls csalt_resource_deinit() on both
 * resources.
 *
 * \sa csalt_resource_pair()
 * \sa csalt_resource_pair_list()
 * \sa csalt_resource_pair_list_bounds()
 */
struct csalt_resource_pair {
	struct csalt_resource_interface *vtable;
	csalt_resource *first;
	csalt_resource *second;
	struct csalt_store_pair result;
};

/**
 * \brief Constructs a csalt_resource_pair.
 */
struct csalt_resource_pair csalt_resource_pair(
	csalt_resource *first,
	csalt_resource *second
);

/**
 * \brief Constructs a linked-list-like structure of csalt_resource_pair%s.
 *
 * Returns 0 on success or -1 on error. Errors include one of the arrays being
 * zero-length, one of the arrays being ill-defined, or the output array being
 * smaller than the input array.
 *
 * \sa csalt_resource_pair
 * \sa csalt_resource_pair_list()
 */
int csalt_resource_pair_list_bounds(
	csalt_resource **resource_begin,
	csalt_resource **resource_end,
	struct csalt_resource_pair *out_begin,
	struct csalt_resource_pair *out_end
);

/**
 * \brief Convenience macro for building a linked-list of csalt_resource_pair%s.
 *
 * This is the recommended way to construct lists of csalt_resource_pair%s with
 * arrays whose lengths are known at compile-time. If the arrays are dynamically
 * allocated, such as with csalt_heap or malloc, you must use
 * csalt_resource_pair_list_bounds() instead.
 *
 * \sa csalt_resource_pair
 * \sa csalt_resource_pair_list_bounds()
 */
#define csalt_resource_pair_list(resources, out) \
	csalt_resource_pair_list_bounds( \
		(resources), \
		arrend(resources), \
		(out), \
		arrend(out) \
	)

csalt_store *csalt_resource_pair_init(csalt_resource *resource);
void csalt_resource_pair_deinit(csalt_resource *resource);

/**
 * \brief This struct uses the first working resource.
 *
 * When initialized, it attempts to initialize the first resource;
 * if that succeeds, it returns that resource. If it fails, this resource
 * moves onto the next and attempts to initialize that.
 *
 * This resource only fails if all resources within it fail
 * to initialize.
 *
 * \see csalt_resource_first_array()
 * \see csalt_resource_first_bounds()
 */
struct csalt_resource_first {
	struct csalt_resource_interface *vtable;
	csalt_resource **begin;
	csalt_resource **end;
	csalt_resource *returned;
};

/**
 * \brief Constructor for a csalt_resource_first_bounds.
 */
struct csalt_resource_first csalt_resource_first_bounds(
	csalt_resource **begin,
	csalt_resource **end
);

/**
 * \brief Convenience macro for constructing a resource_first_bounds
 * from an array.
 */
#define csalt_resource_first_array(array) (csalt_resource_first_bounds((array), &((array)[arrlength(array)])))

csalt_store *csalt_resource_first_init(csalt_resource *resource);
void csalt_resource_first_deinit(csalt_resource *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_COMPOSITERESOURCES_H
