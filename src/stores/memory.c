#include "csalt/stores/memory.h"

#include <string.h>

const struct csalt_static_store_interface mem_impl = {
	csalt_store_memory_read,
	csalt_store_memory_write,
	csalt_store_memory_split,
};

struct csalt_store_memory csalt_store_memory_bounds(void *begin, void *end)
{
	return (struct csalt_store_memory) {
		&mem_impl,
		begin,
		end,
	};
}

ssize_t csalt_store_memory_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t amount
)
{
	struct csalt_store_memory *mem = (void *)store;
	amount = csalt_min(amount, (char*)mem->end - (char*)mem->begin);
	memcpy(buffer, mem->begin, (size_t)amount);
	return amount;
}

ssize_t csalt_store_memory_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t amount
)
{
	struct csalt_store_memory *mem = (void *)store;
	amount = csalt_min(amount, (char*)mem->end - (char*)mem->begin);
	memcpy(mem->begin, buffer, (size_t)amount);
	return amount;
}

int csalt_store_memory_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	struct csalt_store_memory
		*mem = (void*)store,
		tmp = csalt_store_memory_bounds(
			csalt_min((char *)mem->begin + begin, (char*)mem->end),
			csalt_min((char *)mem->begin + end, (char*)mem->end));

	return block((void*)&tmp, param);
}

