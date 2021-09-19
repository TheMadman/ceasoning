#ifndef DECORATORRESOURCES_H
#define DECORATORRESOURCES_H

#include "baseresources.h"
#include "decoratorstores.h"

/**
 * \file
 * \brief This file is responsible for providing decorator functions around
 * csalt_resource%s.
 */

/**
 * \brief Default forwarding function for initializing a resource.
 */
csalt_store *csalt_resource_decorator_init(csalt_resource *);

/**
 * \brief Default fowarding function for deinitializing a resource.
 */
void csalt_resource_decorator_deinit(csalt_resource *);

struct csalt_resource_decorator {
	const struct csalt_resource_interface *vtable;
	csalt_resource *child;

	struct csalt_store_decorator initialized;;
};

#endif // DECORATORRESOURCES_H
