#ifndef CSALTUTIL_H
#define CSALTUTIL_H

/**
 * \file This file provides macros for common tasks
 */

/**
 * Macro for returning the largest of two parameters.
 * This macro performs double-evaluation - you should not
 * pass modifications (such as x++) as parameters.
 */
#define max(a, b) ((a) > (b) ? (a) : (b))

/**
* Macro for returning the smallest of two parameters.
* This macro performs double-evaluation - you should not
* pass modifications (such as x++) as parameters.
*/
#define min(a, b) ((a) > (b) ? (b) : (a))

/**
 * Retrieves the number of elements in a typed array.
 */
#define arrlength(array) (sizeof(array) / sizeof(array[0]))

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

#endif // CSALTUTIL_H
