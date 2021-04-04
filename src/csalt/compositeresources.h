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
 * \brief This resource allows operations on groups of
 * resources.
 *
 * When this resource is passed to csalt_resource_init,
 * it attempts to initialize all resources in the order given.
 * If any of them fail, the initialized resources are
 * deinitialized and the list returns a -1 error code.
 *
 * When this resource is passed to csalt_resource_use,
 * individual resources can be retrieved with
 * csalt_resource_list_get().
 *
 * \see csalt_resource_list_array()
 * \see csalt_resource_list_bounds()
 */
struct csalt_resource_list {
	struct csalt_store_list parent;
};

/**
 * \brief Creates a new csalt_resource_list.
 *
 * This function initializes the list with the given
 * boundaries, and initializes its vtable.
 */
struct csalt_resource_list csalt_resource_list_bounds(
	csalt_resource **begin,
	csalt_resource **end
);

/**
 * \brief Convenience macro for initializing a csalt_resource_list
 * from an array.
 */
#define csalt_resource_list_array(array) (csalt_resource_list_bounds((array), &((array)[arrlength(array)])))

/**
 * \brief Retrieves the resource from the given list at the
 * corresponding index.
 */
csalt_resource *csalt_resource_list_get(
	struct csalt_resource_list *list,
	size_t index
);

void csalt_resource_list_init(csalt_resource *resource);
char csalt_resource_list_valid(const csalt_resource *resource);
void csalt_resource_list_deinit(csalt_resource *resource);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_COMPOSITERESOURCES_H
