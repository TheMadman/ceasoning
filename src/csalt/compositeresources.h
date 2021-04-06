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

struct csalt_resource_list_interface {
	struct csalt_resource_interface parent;
	csalt_store_list_receive_split_fn *receive_split_list;
};

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
	union {
		struct csalt_resource_list_interface *vtable;
		struct csalt_store_list parent;
	};
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

/**
 * \brief Returns the number of resources in this resource list.
 */
size_t csalt_resource_list_length(struct csalt_resource_list *list);

int csalt_resource_list_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

void csalt_resource_list_init(csalt_resource *resource);
char csalt_resource_list_valid(const csalt_resource *resource);
void csalt_resource_list_deinit(csalt_resource *resource);

int csalt_resource_list_receive_split(
	struct csalt_store_list *original,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief Implements the csalt_store_fallback algorithms with
 * csalt_resource_list resource management.
 *
 * Since csalt_store_fallback requires all stores to be valid
 * at the time of check, this resource operates on the assumption
 * that all resources must be initialized and valid before
 * read/write/size/split can take place. Similarly to
 * csalt_resource_list, if any resource fails, the entire list fails.
 *
 * For check-then-fall-back mechanisms on the resource initialization
 * and validation itself, use the csalt_resource_first resource.
 */
struct csalt_resource_fallback {
	union {
		struct csalt_resource_list_interface *vtable;
		struct csalt_store_fallback parent;
	};
};

void csalt_resource_fallback_init(csalt_resource *);

/**
 * \brief Sets up and returns a csalt_resource_fallback.
 */
struct csalt_resource_fallback csalt_resource_fallback_bounds(
	csalt_resource **begin,
	csalt_resource **end
);

/**
 * \brief This struct uses the first working resource.
 *
 * When initialized, it attempts to initialize the first resource;
 * if that succeeds, it wraps that resource. If it fails, this resource
 * moves onto the next and attempts to initialize that.
 *
 * This resource only fails if all resources within it fail
 * to initialize.
 *
 * \see csalt_resource_first_array()
 * \see csalt_resource_first_bounds()
 */
struct csalt_resource_first {
	union {
		struct csalt_resource_interface *vtable;
		struct {
			struct csalt_resource_list parent;
			csalt_resource *initialized;
		};
	};
};

/**
 * \brief Constructor for a csalt_resource_first_bounds.
 */
struct csalt_resource_first csalt_resource_first_bounds(csalt_resource **begin, csalt_resource **end);

/**
 * \brief Convenience macro for constructing a resource_first_bounds
 * from an array.
 */
#define csalt_resource_first_array(array) (csalt_resource_first_bounds((array), &((array)[arrlength(array)])))

ssize_t csalt_resource_first_read(
	const csalt_store *store,
	void *buffer,
	size_t amount
);
ssize_t csalt_resource_first_write(
	csalt_store *store,
	const void *buffer,
	size_t amount
);
size_t csalt_resource_first_size(const csalt_store *store);
int csalt_resource_first_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

void csalt_resource_first_init(csalt_resource *resource);
char csalt_resource_first_valid(const csalt_resource *resource);
void csalt_resource_first_deinit(csalt_resource *resource);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_COMPOSITERESOURCES_H
