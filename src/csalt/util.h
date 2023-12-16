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

#ifndef CSALT_UTIL_H
#define CSALT_UTIL_H

#include <csalt/platform/init.h>

/**
 * \file
 * This file provides macros for common tasks
 */

/**
 * Macro for returning the largest of two parameters.
 * This macro performs double-evaluation - you should not
 * pass modifications (such as x++) as parameters.
 */
#define csalt_max(a, b) ((a) > (b) ? (a) : (b))

/**
* Macro for returning the smallest of two parameters.
* This macro performs double-evaluation - you should not
* pass modifications (such as x++) as parameters.
*/
#define csalt_min(a, b) ((a) > (b) ? (b) : (a))

/**
 * Retrieves the number of elements in a typed array.
 */
#define arrlength(array) (sizeof(array) / sizeof(array[0]))

/**
 * Gets a pointer past the end of the array, such that
 * arrend(array) - array == sizeof(array)
 */
#define arrend(array) (&array[arrlength(array)])

/**
 * \brief A generic array
 */
struct csalt_array {
	/**
	 * \brief A pointer to the beginning of the array
	 */
	void *begin;

	/**
	 * \brief A pointer to one element past the end of
	 * 	the array
	 */
	void *end;

	/**
	 * \brief The size of each member
	 */
	size_t size;
};

/**
 * \public \memberof csalt_array
 * \brief Constructs a csalt_array from a C array.
 */
#define csalt_array(array) ((struct csalt_array) { \
	(array), \
	arrend(array), \
	sizeof(array[0]), \
})

/**
 * \public \memberof csalt_array
 * \brief Performs a liniar search of the array, without
 * 	mutating it, similar to standard lfind.
 */
inline void *csalt_lfind(
	const void *key,
	const struct csalt_array array,
	int (*comp)(const void *, const void *)
)
{
	for (
		char *current = array.begin;
		current <= (char*)array.end;
		current += array.size
	) {
		if (!comp(key, current))
			return current;
	}
	return NULL;
}

#ifdef PAGESIZE
/**
 * DEFAULT_PAGESIZE represents the size of a page if
 * a previous header defines PAGESIZE, or a sensible
 * default for testing if no kernel/system header is
 * included.
 */
#define DEFAULT_PAGESIZE PAGESIZE
#else
#define DEFAULT_PAGESIZE 4096
#endif // PAGESIZE

#endif // CSALT_UTIL_H
