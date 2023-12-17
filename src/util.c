#include "csalt/util.h"

void *csalt_lfind(
	const void *key,
	const struct csalt_array array,
	int (*comp)(const void *, const void *)
)
{
	for (
		char *current = array.begin;
		current < (char*)array.end;
		current += array.size
	) {
		if (!comp(key, current))
			return current;
	}
	return NULL;
}

