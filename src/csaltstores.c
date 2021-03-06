#include "csaltstores.h"

#include <unistd.h>
#include <string.h>

#include "csaltutil.h"

// virtual call functions

ssize_t csalt_store_read(
	const csalt_store *from,
	void *to,
	size_t bytes
)
{
	return (*from)->read(from, to, bytes);
}

ssize_t csalt_store_write(
	csalt_store *to,
	const void *from,
	size_t bytes
)
{
	return (*to)->write(to, from, bytes);
}

int csalt_store_split(
	csalt_store *store,
	size_t start,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	return (*store)->split(store, start, end, block, data);
}

// null interface

ssize_t csalt_store_null_read(
	const csalt_store *from,
	void *to,
	size_t size
)
{
	/*
	 * We create (void) statements to prevent compiler warnings
	 * or errors on unused parameters.
	 *
	 * Those warnings are good to have so writing explicitely that
	 * we want to ignore them will help our compiler help us
	 * more.
	 */
	(void)from;
	(void)to;
	(void)size;
	return -1;
}

ssize_t csalt_store_null_write(
	csalt_store *from,
	const void *to,
	size_t size
)
{
	(void)from;
	(void)to;
	(void)size;
	return -1;
}

int csalt_store_null_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	(void)store;
	(void)begin;
	(void)end;
	(void)block;
	(void)data;

	return 0;
}

const struct csalt_store_interface csalt_store_null_interface = {
	csalt_store_null_read,
	csalt_store_null_write,
	csalt_store_null_split,
};

const struct csalt_store_interface *csalt_store_null_implementation = &csalt_store_null_interface;

// memory interface

ssize_t csalt_memory_read(
	const csalt_store *from,
	void *to,
	size_t size
)
{
	struct csalt_memory *memory = (struct csalt_memory *)from;
	if (memory->end - memory->begin < size)
		return -1;

	memcpy(to, memory->begin, size);
	return size;
}

ssize_t csalt_memory_write(
	csalt_store *to,
	const void *from,
	size_t size
)
{
	struct csalt_memory *memory = (struct csalt_memory *)to;
	if (memory->end - memory->begin < size)
		return -1;

	memcpy(memory->begin, from, size);
	return size;
}

int csalt_memory_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	struct csalt_memory *param = (struct csalt_memory *)store;
	struct csalt_memory result = csalt_memory_bounds(
		param->begin + begin,
		param->begin + end
	);
	return block((csalt_store *)&result, data);
}

struct csalt_memory csalt_memory_bounds(void *begin, void *end)
{
	struct csalt_memory result = {
		&csalt_store_memory_implementation,
		begin,
		end
	};
	return result;
}

