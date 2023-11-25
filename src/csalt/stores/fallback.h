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
/**
 * \file
 */

#ifndef CSALT_FALLBACK_H
#define CSALT_FALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "pair.h"

/**
 * \brief Implements a fallback mechanism for read operations.
 *
 * csalt_store_read() tries reading the first store first.
 * If all the requested data are read, it returns immediately.
 * Otherwise, it attempts to read the remaining data from later
 * stores. If data is read from later stores successfully, it is
 * written back into earlier stores.
 *
 * If any store returns an error code from csalt_store_read(),
 * the whole fallback store halts immediately and the contents
 * of *buffer is undefined.
 *
 * csalt_store_write() writes only to the first store in the list.
 * The data can be written out to all stores by calling
 * csalt_store_fallback_flush() on the fallback.
 *
 * csalt_store_split() behaves similarly to csalt_store_pair, except
 * that the result implements identical fallback logic. Note that
 * the result of the split will be a csalt_static_store_fallback:
 * you _must not_ call csalt_store_size() or csalt_store_resize() on
 * the result.
 *
 * csalt_store_size() implements the same logic as for csalt_store_pair.
 *
 * csalt_store_resize() implements the same logic as for csalt_store_pair.
 */
struct csalt_store_fallback {
	const struct csalt_dynamic_store_interface *vtable;
	struct csalt_store_pair pair;
};

/**
 * \brief Implements a fallback mechanism for read operations.
 *
 * Identical to csalt_store_fallback, but conforming to the
 * csalt_static_store_interface interface.
 */
struct csalt_static_store_fallback {
	const struct csalt_static_store_interface *vtable;
	struct csalt_static_store_pair pair;
};

/**
 * \public \memberof csalt_store_fallback
 *
 * \brief Constructor for a csalt_store_fallback.
 *
 * \param primary The primary store to perform operations
 * 	on.
 * \param fallback The store to fall back on, should a read
 * 	or write fail on the primary store.
 *
 * \returns A constructed csalt_store_fallback.
 */
struct csalt_store_fallback csalt_store_fallback(
	csalt_store *primary,
	csalt_store *fallback);

/**
 * \public \memberof csalt_static_store_fallback
 *
 * \brief Constructor for a csalt_static_store_fallback.
 *
 * \param primary The primary store to perform operations
 * 	on.
 * \param fallback The store to fall back on, should a read
 * 	or write fail on the primary store.
 *
 * \returns A constructed csalt_store_fallback. 
 */
struct csalt_static_store_fallback csalt_static_store_fallback(
	csalt_static_store *primary,
	csalt_static_store *fallback);


/**
 * \public \memberof csalt_static_store_fallback
 */
ssize_t csalt_store_fallback_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount);

/**
 * \public \memberof csalt_static_store_fallback
 */
ssize_t csalt_store_fallback_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount);

/**
 * \public \memberof csalt_static_store_fallback
 */
int csalt_store_fallback_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_store_block_fn *block,
	void *param);

/**
 * \public \memberof csalt_store_fallback
 */
ssize_t csalt_store_fallback_size(csalt_store *store);

/**
 * \public \memberof csalt_store_fallback
 */
ssize_t csalt_store_fallback_resize(
	csalt_store *store,
	ssize_t new_size);

/**
 * \public \memberof csalt_static_store_fallback
 * \brief Applies the primary store's latest state
 * 	to the fallback store.
 *
 * \param store The csalt_static_store_fallback or
 * 	csalt_store_fallback to apply this to.
 * \param amount The amount of data to write back.
 *
 * \returns The amount of bytes written.
 */
ssize_t csalt_store_fallback_flush(
	struct csalt_static_store_fallback *store,
	ssize_t amount);

/**
 * \public \memberof csalt_store_fallback
 * \brief Initializes an array of csalt_store_fallback's
 * 	from an array of stores, in a linked-list
 * 	configuration.
 *
 * Similar to the csalt_store_pair_list_bounds().
 *
 * For statically allocated arrays, csalt_store_fallback_list()
 * provides a convenience macro for calling this function.
 *
 * This function should only be used directly for
 * dynamically-allocated arrays.
 *
 * \param begin The beginning of the array of stores
 * \param end The end of the array of stores
 * \param out_begin The beginning of the output array
 * \param out_end The end of the output array
 *
 * \returns 0 on success, -1 on error.
 */
int csalt_store_fallback_array_bounds(
	csalt_store **begin,
	csalt_store **end,
	struct csalt_store_fallback *out_begin,
	struct csalt_store_fallback *out_end
);

#define csalt_store_fallback_array(array, out_array) \
	csalt_store_fallback_array_bounds( \
		(array), \
		arrend(array), \
		(out_array), \
		arrend(out_array))

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_FALLBACK_H 
