#ifndef CSALT_COMPOSITESTORES_H
#define CSALT_COMPOSITESTORES_H

/**
 * \file 
 * \brief This file provides stores which define relationships
 * between stores. Examples include csalt_store_fallback, which allows you to
 * define stores as a priority list where each is tried in order.
 */

#include <csalt/basestores.h>
#include <csalt/util.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct csalt_store_list;

/**
 * This function type is the kind of function called by the generic csalt_store_list
 * interface when csalt_store_split() is called on it. It allows the logic of
 * splitting the list to be contained in csalt_store_list, while different kinds
 * of list only have to initialize a stack variable using the result.
 */
typedef int csalt_store_list_receive_split_fn(
	struct csalt_store_list *origina,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

struct csalt_store_list_interface {
	struct csalt_store_interface parent;
	csalt_store_list_receive_split_fn *receive_split_list;
};

/**
 * \brief Most abstract stores are implemented as csalt_store_lists, with different
 * algorithms for iterating over the elements in the list.
 *
 * Take care that the stores placed in lists are of similar size, or capable
 * of reading/writing a similar amount of data by some other means.
 */
struct csalt_store_list {
	struct csalt_store_list_interface *vtable;
	csalt_store **begin;
	csalt_store **end;
};

/**
 * Convenience macro for casting to a csalt_store_list *
 */
#define csalt_store_list(param) castto(struct csalt_store_list *, (param))

struct csalt_store_list csalt_store_list_bounds(
	csalt_store **begin,
	csalt_store **end
);

#define csalt_store_list_array(array) (csalt_store_list_bounds(array, (&array[arrlength(array)])))

/**
 * \brief Returns the store at the given index.
 *
 * Provides a bounds-checked getter for the elements
 * in the list.
 */
csalt_store *csalt_store_list_get(
	const struct csalt_store_list *store,
	size_t index
);

/**
 * \brief Returns the number of stores in this list store.
 */
size_t csalt_store_list_length(const struct csalt_store_list *store);

ssize_t csalt_store_list_read(const csalt_store *store, void *buffer, size_t amount);
ssize_t csalt_store_list_write(csalt_store *store, const void *buffer, size_t amount);
int csalt_store_list_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);
size_t csalt_store_list_size(const csalt_store *store);
int csalt_store_list_receive_split(
	struct csalt_store_list *original,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

/**
 * \brief The csalt_store_fallback_store is a csalt_store_list providing
 * fallback logic for operations on the stores.
 *
 * The behaviour of this store is intended to mimick caching logic.
 * It prevents reading/writing from later stores if data was found
 * previously, allowing you to skip slow stores if a faster store holds
 * the correct data.
 *
 * On csalt_store_read(), the read is first attempted on the first item in the
 * list. If that item returns the amount expected, the whole fallback
 * returns; otherwise, the next item is attempted. The fallback store
 * keeps iterating through the list until a store reads the expected
 * amount of bytes, or the list is exhausted.
 *
 * If complete data is found in a store, it is written to the stores
 * that came before it.
 *
 * If the store reaches the end of the list without reading the full
 * amount of bytes expected, it reads from the store which returned
 * the most bytes and returns the amount of bytes read from that store.
 *
 * Standard csalt_store_write() calls only write to the first store
 * in the list. To write-out the contents of the last write to the
 * rest of the stores, you can use csalt_store_fallback_flush().
 * More nuanced write behaviour can be achieved by writing to
 * individual stores retrieved with csalt_store_list_get().
 *
 * The csalt_store_size() call returns the smallest reported size
 * of all the stores, allowing for safe reads and writes. 
 *
 * Calling csalt_store_split() on a fallback store provides a fallback
 * store in which every store it contains is split by the given amount.
 * This requires at least one dynamic memory allocation.
 */
struct csalt_store_fallback {
	struct csalt_store_list list;
	size_t amount_written;
};

/**
 * \brief Constructor for csalt_store_fallback taking a range of
 * pointers.
 */
struct csalt_store_fallback csalt_store_fallback_bounds(
	csalt_store **begin,
	csalt_store **end
);

/**
 * \brief Convenience macro for initializing a fallback store from an
 * array.
 */
#define csalt_store_fallback_array(array) csalt_store_fallback_bounds((array), (&array[arrlength(array)]))

ssize_t csalt_store_fallback_read(
	const csalt_store *store,
	void *buffer,
	size_t size
);

ssize_t csalt_store_fallback_write(
	csalt_store *store,
	const void *buffer,
	size_t size
);

int csalt_store_fallback_receive_split(
	struct csalt_store_list *original,
	struct csalt_store_list *list,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
);

size_t csalt_store_fallback_size(const csalt_store *store);

/**
 * \brief Writes out data from the first store into all stores
 * after it.
 *
 * Takes an array of csalt_transfers, one for each store
 * in the fallback, which can safely be initialized to zero:
 *
 * \code
 * 	// Outside your main loop
 * 	// Assume number_stores is a const and you know the list's length
 * 	// otherwise, this becomes a heap allocation
 * 	// size_t number_stores = csalt_store_list_length(&fallback_ptr->list);
 * 	struct csalt_transfer transfers[number_stores] = { 0 };
 *
 * 	// inside your main loop, or in a while loop for blocking behaviour
 * 	struct csalt_transfer total_progress;
 * 	total_progress = csalt_store_fallback_flush(fallback_ptr, transfers);
 * \endcode
 *
 * As an academic note, you can actually safely use `number_stores - 1`
 * csalt_transfers, since you're not transferring from the first store
 * to the rest - but this is easier to remember.
 *
 */
struct csalt_transfer csalt_store_fallback_flush(
	struct csalt_store_fallback *store,
	struct csalt_transfer *transfers
);

#ifdef __cplusplus
}
#endif

#endif // CSALT_COMPOSITESTORES_H
