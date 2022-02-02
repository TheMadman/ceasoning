#ifndef CSALT_KEYSTORES
#define CSALT_KEYSTORES

/**
 * \file
 * \brief This file provides interfaces for associative stores.
 *
 * An associative store is any store which can store or retrieve a value by
 * key. This can range from a hashtable to a database to a REST API.
 */

#include "stores.h"

#ifdef __cplusplus
extern "C" {
#endif

struct csalt_keystore_interface;

/**
 * \brief A keystore publishes two methods - csalt_keystore_set and
 * 	csalt_keystore_get - which allow you to store and retrieve
 * 	values by a given key.
 *
 * The built-in keystores have a few limitations:
 *
 * The key and value stores should contain the full data -
 * in other words, they should have no pointers to external memory:
 *
 * \code
 *
 *	// good
 *	struct csalt_store_memory key = csalt_store_memory_array("Key");
 *
 *	// bad
 *	struct {
 *		const char *firstname;
 *		const char *lastname;
 *	} key = { ... };
 *	struct csalt_store_memory value = csalt_store_memory_pointer(&key);
 *
 * \endcode
 */
typedef const struct csalt_keystore_interface *csalt_keystore;

typedef int csalt_keystore_set_fn(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store *value
);

typedef int csalt_keystore_get_fn(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store_block_fn *block,
	void *param
);

struct csalt_keystore_interface {
	csalt_keystore_set_fn *set;
	csalt_keystore_get_fn *get;
};

/**
 * \brief This function associates a key with a value in the given keystore.
 *
 * For the built-in keystores, both the key and the value should be totally
 * contained within the store: they should NOT contain pointers to external
 * memory or similar constructs, such as index offsets and the like.
 *
 * If you have a store which contains multiple values, such as an array or
 * file, and you only want to associate one value with the given key, you
 * should first split the store such that it begins where the value begins
 * and ends where the value ends.
 */
int csalt_keystore_set(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store *value
);

/**
 * \brief This function searches the keystore for the given key. If the key
 * 	is found, the value and \c param are passed to \c block. Otherwise,
 * 	\c block isn't called.
 *
 * The \c csalt_store passed to \c block can have its size queried to
 * receive the byte size of the value, and it can be read from to retrieve
 * the value. Writing to the passed store is undefined.
 *
 * If the key wasn't found or there was an error with the keystore, -1 is
 * returned. Otherwise, the return value from \c block is returned.
 */
int csalt_keystore_get(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store_block_fn *block,
	void *param
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSALT_KEYSTORES
