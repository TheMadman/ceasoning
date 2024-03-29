/*
 * Ceasoning - Syntactic Sugar for Common C Tasks
 * Copyright (C) 2022   Marcus Harrison
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSALT_COMPOSITESTORES_H
#define CSALT_COMPOSITESTORES_H

#include <csalt/platform/init.h>

/**
 * \file
 * \brief This file provides stores which define relationships
 * between stores. Examples include csalt_store_pair, which can be used
 * to create pairs, linked-lists or binary trees of stores, and
 * csalt_store_fallback, which allows you to define stores as a priority
 * list where each store is tried in order.
 */

#include <csalt/basestores.h>
#include <csalt/util.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This type allows storing a pair of stores and interacting
 * 	with them as though they are a single store.
 *
 * This type can be used to represent coupled stores, as well as lists
 * of stores using a linked-list-like approach or binary trees using
 * a nested approach. Either member may be initialized to a null
 * pointer.
 *
 * A constructor is available for creating an individual pair, as well
 * as one for initializing an array of pairs for a list of stores.
 *
 * csalt_store_read() attempts a read from the first store. If that
 * store returns zero bytes, the second store is tried instead. If either
 * store returns an error, this method returns an error and the contents
 * of \c *buffer is undefined.
 *
 * csalt_store_write() attempts to write to the first store first. If
 * there was an error, it returns an error immediately; otherwise, it
 * then attempts to write to the second store. If that errors, the whole
 * pair returns an error and the data in the first store is undefined. If
 * the write was successful on both stores, the pair returns the lowest
 * amount written, allowing for repeated attempts without data loss.
 *
 * csalt_store_size() returns the smallest size reported by the two
 * stores.
 *
 * csalt_store_split() passes a new pair, whose members have both
 * been split by the requested amount.
 *
 * \sa csalt_store_pair()
 * \sa csalt_store_pair_list()
 */
struct csalt_store_pair {
	const struct csalt_store_interface *interface;
	csalt_store *first;
	csalt_store *second;
};

/**
 * \brief Constructor for creating a single pair.
 */
struct csalt_store_pair csalt_store_pair(csalt_store *first, csalt_store *second);

ssize_t csalt_store_pair_read(csalt_store *store, void *buffer, ssize_t size);
ssize_t csalt_store_pair_write(csalt_store *store, const void *buffer, ssize_t size);
ssize_t csalt_store_pair_size(csalt_store *store);
int csalt_store_pair_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

/**
 * \brief Constructor for creating a list of pairs, given
 * 	an array of csalt_store%s to link together.
 *
 * This function effectively constructs a linked-list from pairs.
 * The first pair's first property points to the first store in
 * the array. The second property points to the second pair.
 * The second pair's first property points to the second store,
 * and its second property points to the next pair, and so on.
 * The last pair's second property is set to a null pointer value.
 *
 * Note that this function uses an out-parameter for initializing an
 * already-existing array, instead of returning a value.
 *
 * Constructing an array for an existing local array of stores is simple:
 * \code
 *
 * 	struct csalt_store_pair list[arrsize(stores)] = { 0 };
 * \endcode
 *
 * The output array is untouched if there was an error with the
 * parameters, such as the input/output being zero length, or the
 * output array being smaller than the input array
 *
 * \param begin The beginning of the array of stores
 * \param end The end of the array of stores
 *
 * \sa csalt_store_pair_list()
 */
void csalt_store_pair_list_bounds(
	csalt_store **begin,
	csalt_store **end,
	struct csalt_store_pair *out_array_begin,
	struct csalt_store_pair *out_array_end
);

/**
 * \brief Convenience macro for constructing a pair_list from two arrays.
 *
 * This macro is the recommended way of constructing a pair_list, given
 * the two arrays' sizes are defined at compile-time. Dynamic arrays, such as
 * those allocated with malloc() or calloc(), must still use
 * csalt_store_pair_list_bounds().
 *
 * \sa csalt_store_pair_list_bounds()
 */
#define csalt_store_pair_list(store_array, out_array) \
	csalt_store_pair_list_bounds( \
		(store_array), \
		arrend(store_array), \
		out_array, \
		arrend(out_array) \
	)

/**
 * \brief Allows checking the length of a list of pairs constructed
 * 	by csalt_store_pair_list() or csalt_store_pair_list_bounds().
 */
ssize_t csalt_store_pair_list_length(const struct csalt_store_pair *pairs);

/**
 * \brief Allows getting the csalt_store object at an offset from a
 * 	list of pairs constructed by csalt_store_pair_list() or
 * 	csalt_store_pair_list_bounds().
 */
csalt_store *csalt_store_pair_list_get(
	const struct csalt_store_pair *pairs,
	ssize_t index
);

/**
 * \brief Defines a single split for the csalt_store_pair_list_multisplit()
 * 	function.
 *
 * An array of these is passed to csalt_store_pair_list_multisplit() to define
 * the bounds of each store to be split.
 *
 * \sa csalt_store_pair_list_multisplit_bounds()
 * \sa csalt_store_pair_list_multisplit()
 */
struct csalt_store_multisplit_split {
	/**
	 * \brief Where the split should begin. Identical to the
	 * 	`begin` param in csalt_store_split().
	 */
	ssize_t begin;

	/**
	 * \brief Where the split should end. Identical to the
	 * 	`end` param in csalt_store_split().
	 */
	ssize_t end;
};

/**
 * \brief Split multiple stores using different bounds per store.
 *
 * This function takes a linked list of stores, as constructed by
 * csalt_store_pair_list(), an array of csalt_store_multisplit_split
 * structs defining how to split each item in the list, and finally,
 * the `block` and `param` parameters identically to csalt_store_split().
 *
 * Only `param` may be null; setting any other parameter to null is
 * undefined.
 *
 * The first struct in the csalt_store_multisplit_split array defines
 * how to split the first store; the second struct, the second store; and
 * so on.
 *
 * If the csalt_store_multisplit_split array is smaller than the list,
 * only the first elements of the list are split; the rest are appended
 * as-is. If the csalt_store_multisplit_split array is larger than the
 * list, the remainders in the array are ignored.
 *
 * Once the stores are split, they are wrapped in a new linked list
 * and passed to `block`.
 *
 * Example:
 *
 * \code
 *
 * 	int my_function(csalt_store *, void *);
 * 	csalt_store *stores[] = {
 * 		(csalt_store *)first_store,
 * 		(csalt_store *)second_store,
 * 		(csalt_store *)third_store,
 * 	};
 *
 * 	struct csalt_store_pair list[arrlength(stores)] = { 0 };
 * 	csalt_store_pair_list(stores, list);
 *
 * 	struct csalt_store_multisplit_split splits[] = {
 * 		{ 0, 3 },
 * 		{ 4, 8 },
 * 	};
 *
 * 	csalt_store_pair_list_multisplit(list, splits, my_function, NULL);
 *
 * \endcode
 *
 * This will split `first_store` from 0 to 3, `second_store` from 4 to 8,
 * leave `third_store` as-is, wrap them in a new `csalt_store_pair_list` and
 * pass the result as `my_function`'s first parameter.
 *
 * \sa csalt_store_multisplit_split
 * \sa csalt_store_pair_list_multisplit()
 */
int csalt_store_pair_list_multisplit_bounds(
	struct csalt_store_pair *list,
	const struct csalt_store_multisplit_split *begin,
	const struct csalt_store_multisplit_split *end,
	csalt_store_block_fn *block,
	void *param
);

/**
 * \brief Convenience macro for csalt_store_pair_list_multisplit_bounds().
 *
 * Takes a static array as the second argument, instead of separate begin/end
 * pointers.
 *
 * \sa csalt_store_pair_list_multisplit_bounds()
 */
#define csalt_store_pair_list_multisplit(list, multisplit_split_arr, block, param) \
	csalt_store_pair_list_multisplit_bounds( \
		list, \
		multisplit_split_arr, \
		arrend(multisplit_split_arr), \
		block, \
		param)

/**
 * \brief This type decorates a pair with fallback/caching logic.
 *
 * csalt_store_read() tries reading the first store first. If all the requested
 * data are read, it returns immediately. Otherwise, it attempts to read the
 * remaining data from later stores. If data is read from the later stores
 * successfully, it is written back into earlier stores.
 *
 * If any store returns an error code from csalt_store_read(), the whole
 * fallback store halts immediately and the contents of *buffer is undefined.
 *
 * csalt_store_write() writes only to the first store in the list. The data
 * can be written out to all stores by calling csalt_store_flush() on the list.
 *
 * csalt_store_size() implements the same logic as for csalt_store_pair.
 *
 * csalt_store_split() behaves similarly to csalt_store_pair, except the
 * result implements the same fallback logic for csalt_store_read() and
 * csalt_store_write().
 *
 * \sa csalt_store_fallback_array()
 * \sa csalt_store_fallback_bounds()
 */
struct csalt_store_fallback {
	const struct csalt_store_interface *vtable;
	struct csalt_store_pair pair;
};

/**
 * \brief Constructs a csalt_store_fallback from two csalt_store%s.
 */
struct csalt_store_fallback csalt_store_fallback(
	csalt_store *first,
	csalt_store *second
);

/**
 * \brief This function constructs a list of stores which implements caching
 * 	logic.
 *
 * The arguments should be the same as for the csalt_store_pair_list_bounds(),
 * except taking an out-array of csalt_store_fallback as its last arguments.
 *
 * \sa csalt_store_fallback_array()
 * \sa csalt_store_pair_list_bounds()
 */
int csalt_store_fallback_bounds(
	csalt_store **list_begin,
	csalt_store **list_end,
	struct csalt_store_fallback *fallback_begin,
	struct csalt_store_fallback *fallback_end
);

/**
 * \brief Convenience macro from building csalt_store_fallback%s from two
 * 	arrays.
 *
 * This is the recommended way to construct csalt_store_fallback%s when the
 * array lengths are known at compile-time.
 *
 * \sa csalt_store_fallback_bounds()
 */
#define csalt_store_fallback_array(store_array, fallback_array) \
	csalt_store_fallback_bounds( \
		(store_array), \
		arrend(store_array), \
		(fallback_array), \
		arrend(fallback_array) \
	)

ssize_t csalt_store_fallback_read(
	csalt_store *store,
	void *buffer,
	ssize_t amount
);

ssize_t csalt_store_fallback_write(
	csalt_store *store,
	const void *buffer,
	ssize_t amount
);

ssize_t csalt_store_fallback_size(csalt_store *store);

int csalt_store_fallback_split(
	csalt_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param
);

#ifdef __cplusplus
}
#endif

#endif // CSALT_COMPOSITESTORES_H
