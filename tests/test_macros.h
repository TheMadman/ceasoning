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
	size_t last_read;
	ssize_t last_write;
	size_t split_begin;
	size_t split_end;
};


ssize_t csalt_store_stub_read(const csalt_store *store, void *data, size_t amount)
{
	(void)data;
	struct csalt_store_stub *stub = (void *)store;
	stub->last_read = min((ssize_t)amount, stub->size);
	return stub->last_read;
}

ssize_t csalt_store_stub_write(csalt_store *store, const void *data, size_t amount)
{
	(void)data;
	struct csalt_store_stub *stub = (void *)store;
	stub->last_write = min((ssize_t)amount, stub->size);
	return stub->last_write;
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
	struct csalt_store_stub *stub = (void *)store;
	stub->split_begin = begin;
	stub->split_end = end;
	int result = block(store, data);
	stub->split_begin = 0;
	stub->split_end = 0;
	return result;
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

ssize_t csalt_store_stub_error_read(const csalt_store *_, void *__, size_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

ssize_t csalt_store_stub_error_write(csalt_store *_, const void *__, size_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return -1;
}

size_t csalt_store_stub_error_size(const csalt_store *_)
{
	return 0;
}

struct csalt_store_interface csalt_store_stub_error_implementation = {
	csalt_store_stub_error_read,
	csalt_store_stub_error_write,
	csalt_store_stub_error_size,
	csalt_store_stub_split,
};

struct csalt_store_stub csalt_store_stub_error()
{
	struct csalt_store_stub result = {
		&csalt_store_stub_error_implementation,
	};
	return result;
}

ssize_t csalt_store_stub_zero_read(const csalt_store *_, void *__, size_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return 0;
}

ssize_t csalt_store_stub_zero_write(csalt_store *_, const void *__, size_t ___)
{
	(void)_;
	(void)__;
	(void)___;
	return 0;
}

struct csalt_store_interface csalt_store_stub_zero_implementation = {
	csalt_store_stub_zero_read,
	csalt_store_stub_zero_write,
	csalt_store_stub_error_size,
	csalt_store_stub_split,
};

struct csalt_store_stub csalt_store_stub_zero()
{
	struct csalt_store_stub result = {
		&csalt_store_stub_zero_implementation,
	};
	return result;
}

csalt_store *csalt_resource_stub_init_fail(csalt_resource *resource)
{
	(void)resource;
	return 0;
}

csalt_store *csalt_resource_stub_init_fail(csalt_resource *);
csalt_store *csalt_resource_stub_init_success(csalt_resource *);
void csalt_resource_stub_deinit(csalt_resource *);

struct csalt_resource_interface csalt_resource_stub_fail_implementation = {
	csalt_resource_stub_init_fail,
	csalt_resource_stub_deinit,
};

struct csalt_resource_stub {
	struct csalt_resource_interface *vtable;
	struct csalt_store_stub return_value;
};

csalt_store *csalt_resource_stub_init_success(csalt_resource *resource)
{
	struct csalt_resource_stub *stub = (void *)resource;
	return (csalt_store *)&stub->return_value;
}

void csalt_resource_stub_deinit(csalt_resource *resource)
{
	(void)resource;
}

struct csalt_resource_interface csalt_resource_stub_succeed_implementation = {
	csalt_resource_stub_init_success,
	csalt_resource_stub_deinit,
};

struct csalt_resource_stub csalt_resource_stub(size_t size)
{
	struct csalt_resource_stub result = {
		&csalt_resource_stub_succeed_implementation,
		csalt_store_stub(size),
	};

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
