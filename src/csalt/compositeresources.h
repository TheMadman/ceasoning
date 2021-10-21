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
