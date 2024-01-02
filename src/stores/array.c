#include "csalt/stores/array.h"

#include "csalt/stores/base.h"

typedef struct csalt_store_array array_t;

static const struct csalt_dynamic_store_interface impl = {
	{
		csalt_store_array_read,
		csalt_store_array_write,
		csalt_store_array_split,
	},
	csalt_store_array_size,
	csalt_store_array_resize,
};

struct csalt_store_array csalt_store_array(
	csalt_store *store,
	ssize_t object_size
)
{
	return (struct csalt_store_array) {
		{
			.vtable = &impl,
			.decorated = store,
		},
		.object_size = object_size,
	};
}

ssize_t csalt_store_array_read(
	csalt_static_store *store,
	void *buffer,
	ssize_t size
)
{
	array_t *array = (array_t*)store;
	return csalt_store_read(
			array->parent.decorated_static,
			buffer,
			size * array->object_size)
		/ array->object_size;
}

ssize_t csalt_store_array_write(
	csalt_static_store *store,
	const void *buffer,
	ssize_t size
)
{
	array_t *array = (array_t*)store;
	return csalt_store_write(
			array->parent.decorated_static,
			buffer,
			size * array->object_size)
		/ array->object_size;
}

int csalt_store_array_split(
	csalt_static_store *store,
	ssize_t begin,
	ssize_t end,
	csalt_static_store_block_fn *block,
	void *param
)
{
	array_t *array = (array_t*)store;
	begin *= array->object_size;
	end *= array->object_size;
	return csalt_store_split(
		array->parent.decorated_static,
		begin,
		end,
		block,
		param);
}

ssize_t csalt_store_array_size(csalt_store *store)
{
	array_t *array = (array_t*)store;
	return csalt_store_size(array->parent.decorated)
		/ array->object_size;
}

ssize_t csalt_store_array_resize(csalt_store *store, ssize_t new_size)
{
	array_t *array = (array_t*)store;
	return csalt_store_resize(
			array->parent.decorated,
			new_size * array->object_size)
		/ array->object_size;
}

struct array_set {
	const ssize_t object_size;
	const void *const param;

	ssize_t result;
};

static int receive_for_set(csalt_static_store *store, void *param)
{
	struct array_set *set = param;
	set->result = csalt_store_write(store, set->param, set->object_size);
	return !!set->result;
}

bool csalt_store_array_set(
	struct csalt_store_array *array,
	ssize_t index,
	const void *buffer
)
{
	struct array_set set = {
		array->object_size,
		buffer,

		-1
	};

	const ssize_t begin = index * array->object_size;
	const ssize_t end = begin + array->object_size;

	const int success = csalt_store_split(
		array->parent.decorated_static,
		begin,
		end,
		&receive_for_set,
		&set);

	if (!success)
		return false;

	// set->result is written by receive_for_set
	if (set.result != array->object_size)
		return false;

	return true;
}

struct array_get {
	const ssize_t object_size;
	void *const param;

	ssize_t result;
};

static int receive_for_get(csalt_static_store *store, void *param)
{
	struct array_get *get = param;
	get->result = csalt_store_read(store, get->param, get->object_size);
	return !!get->object_size;
}

bool csalt_store_array_get(
	const struct csalt_store_array *array,
	ssize_t index,
	void *buffer
)
{
	struct array_get get = {
		array->object_size,
		buffer,

		-1
	};

	const ssize_t begin = index * array->object_size;
	const ssize_t end = begin + array->object_size;

	const int success = csalt_store_split(
		array->parent.decorated_static,
		begin,
		end,
		&receive_for_get,
		&get);

	if (!success)
		return false;

	if (get.result != array->object_size)
		return false;

	return true;
}


