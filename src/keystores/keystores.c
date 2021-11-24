#include <csalt/keystores.h>

int csalt_keystore_set(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store *value
)
{
	return (*keystore)->set(keystore, key, value);
}

int csalt_keystore_get(
	csalt_keystore *keystore,
	csalt_store *key,
	csalt_store_block_fn *block,
	void *param
)
{
	return (*keystore)->get(keystore, key, block, param);
}

