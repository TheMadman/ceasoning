#ifndef TEST_MACROS_H
#define TEST_MACROS_H

#include <stdio.h>
#include <csalt/resources.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_TEST_ERROR 99
#define EXIT_TEST_SKIPPED 77
#define print_error(...) do {	\
	fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);	\
	fprintf(stderr, __VA_ARGS__);	\
	fprintf(stderr, "\n");	\
} while (0)


struct csalt_store_stub {
	struct csalt_store_interface *vtable;
	size_t size;
};


ssize_t csalt_store_stub_read(const csalt_store *store, void *data, size_t amount)
{
	(void)store;
	(void)data;
	return amount;
}

ssize_t csalt_store_stub_write(csalt_store *store, const void *data, size_t amount)
{
	(void)store;
	(void)data;
	return amount;
}

size_t csalt_store_stub_size(const csalt_store *store)
{
	const struct csalt_store_stub *stub = (void *)store;
	return stub->size;
}

int csalt_store_stub_split(
	csalt_store *store,
	size_t begin,
	size_t end,
	csalt_store_block_fn *block,
	void *data
)
{
	return block(store, data);
}

struct csalt_store_interface csalt_store_stub_interface = {
	csalt_store_stub_read,
	csalt_store_stub_write,
	csalt_store_stub_size,
	csalt_store_stub_split
};

struct csalt_store_stub csalt_store_stub(size_t size)
{
	struct csalt_store_stub result = {
		&csalt_store_stub_interface,
		size
	};
	return result;
}

csalt_resource_initialized *csalt_resource_stub_init_fail(csalt_resource *resource)
{
	(void)resource;
	return 0;
}

struct csalt_resource_interface csalt_resource_stub_fail_implementation = {
	csalt_resource_stub_init_fail,
};

struct csalt_resource_stub {
	struct csalt_resource_interface *vtable;
	union {
		struct csalt_resource_initialized_interface *return_value_vtable;
		struct csalt_store_stub return_value;
	};
};

csalt_resource_initialized *csalt_resource_stub_init_success(csalt_resource *resource)
{
	struct csalt_resource_stub *stub = (void *)resource;
	return &stub->return_value_vtable;
}

void csalt_resource_stub_deinit(csalt_resource_initialized *resource)
{
	(void)resource;
}

struct csalt_resource_interface csalt_resource_stub_succeed_implementation = {
	csalt_resource_stub_init_success,
};

struct csalt_resource_initialized_interface csalt_resource_stub_initialized_implementation = {
	{
		csalt_store_stub_read,
		csalt_store_stub_write,
		csalt_store_stub_size,
		csalt_store_stub_split,
	},
	csalt_resource_stub_deinit,
};

struct csalt_resource_stub csalt_resource_stub(size_t size)
{
	struct csalt_resource_stub result = {
		&csalt_resource_stub_succeed_implementation,
		{
			.return_value = csalt_store_stub(size),
		},
	};

	result.return_value_vtable = &csalt_resource_stub_initialized_implementation;

	return result;
}

struct csalt_resource_stub csalt_resource_stub_fail()
{
	struct csalt_resource_stub result = {
		&csalt_resource_stub_fail_implementation,
	};

	return result;
}

#endif // TEST_MACROS_H
